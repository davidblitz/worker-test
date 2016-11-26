Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
  
    // Update s_js_ready on watch
  Pebble.sendAppMessage({'JSReady': 1});
  
  Pebble.sendAppMessage({'Start': 3})
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) { 
    var dict = e.payload
    
    console.log('AppMessage received - current counter =  ' + dict['CurrentCount']);
  }                     
);