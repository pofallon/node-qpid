{
  "targets": [
    {
      "target_name": "cproton",
      "sources": [ "src/cproton.cc", "src/messenger.cc" ],
      "link_settings" : {
        "libraries": [ '-lqpid-proton' ]
      }
    }
  ]
}
