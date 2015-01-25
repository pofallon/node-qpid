class install_proton {
	include apt
	include stdlib

	exec { 'apt-get update':
		command => '/usr/bin/apt-get update'
	} ->
	apt::ppa { 'ppa:qpid/released': 
	} ->
	package { 'libqpid-proton2-dev': 
		ensure => installed
	}
}

class install_nodejs { 
	include apt
	include stdlib

	package { ['curl', 'build-essential']:
		ensure => installed
	} ->
	exec { 'add_repo': 
		command => 'curl -sL https://deb.nodesource.com/setup | bash - ; /usr/bin/apt-get update',
		path => ['/usr/bin', '/bin'], 
		creates => '/etc/apt/sources.list.d/nodesource.list'
	} ->
	class { 'nodejs': 
	} ->
	package { 'node-gyp': 
		ensure => installed,
		provider => 'npm'
	}
}

class setup_node_qpid {

	file { '/home/vagrant/node-qpid': 
		ensure => 'link',
		target => '/vagrant'
	} ->
	exec {'rebuild-node-gyp': 
		command => '/usr/bin/node-gyp rebuild',
		cwd => '/home/vagrant/node-qpid',
		environment => ["HOME=/home/vagrant"]
	} ->
	exec {'npm-install':
		command => '/usr/bin/npm install',
		cwd => '/home/vagrant/node-qpid'
	}
}

class test_node_qpid {
	class { 'rabbitmq':
		config_variables => {
			'loopback_users' => '[]'
		}
	} ->
	rabbitmq_plugin {'rabbitmq_amqp1_0': 
		ensure => present
	}
	# class { 'activemq':
	# }
}

include install_proton
include install_nodejs
include setup_node_qpid
include test_node_qpid

Class['install_proton'] -> Class['install_nodejs'] -> Class['setup_node_qpid'] -> Class['test_node_qpid']