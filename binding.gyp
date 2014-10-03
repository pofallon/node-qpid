{
  "targets": [
    {
      "target_name": "cproton",
      "sources": [ "src/cproton.cc", "src/messenger.cc" ],
      "link_settings" : {
        "libraries": [ '-lqpid-proton' ],
        "library_dirs": [ 'C:/dev/tools/qpid/qpid-proton-0.7/build/proton-c/Release' ]
      },
      "include_dirs": [ 'c:/dev/tools/qpid/qpid-proton-0.7/proton-c/include' ]
    }
  ]
}
