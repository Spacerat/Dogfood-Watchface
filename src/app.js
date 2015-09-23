
Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://dogfood-config.apps.veryjoe.com';
  console.log('Showing configuration page: ' + url);
  Pebble.openURL(url);
});


Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  if (configData.bgColor) {
    Pebble.sendAppMessage({
      bgCol: parseInt(configData.bgColor, 16),
      handCol: parseInt(configData.handColor, 16),
      textCol: parseInt(configData.textColor, 16)
    }, function() {
      console.log('Settings sent successfuly!');
    }, function() {
      console.log('Settings failed to send!');
    });
  }
});