{
  "targets": [
    {
      "target_name": "cproton",
      "type": "loadable_module",
      "sources": [ "src/cproton.cc", "src/messenger.cc" ],
      "link_settings" : {
        "libraries": [ '-lqpid-proton' ],
        "library_dirs": [ '/usr/local/lib' ]
      },
      "include_dirs": [ 
        '/usr/local/include/proton', 
        '/usr/include/nodejs/src',
        '/usr/include/nodejs/deps/uv/include/',
        '/usr/include/nodejs/deps/v8/include/',
      ],
      "cflags": [
        "-fPIC"
      ]
    }
  ]
}
