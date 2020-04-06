A NAPI Native C++ Addons to monitor Windows Registry changes using [RegNotifyChangeKeyValue](https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regnotifychangekeyvalue)

Example
=======

```js

const regmon = require("regmon");

let monitor = regmon("HKCU","Software/Microsoft/Windows/CurrentVersion/Run",function(){
  console.log("Registry change detected !");
  monitor.close(); //stop monitoring
});

```

API
===

function(hkey, subkey, callback)

hkey: Root hive key, possible values are HKCR, HKCU, HKLM, HKU, HCC<br/>
subkey: Registry subkey<br/>
callback: Callback function to execute on monitored registry key change<br/>

Return an object containing a close method to stop the monitoring

Installation
============

`npm install regmon`

You will need C/C++ build tools and Python 2.7(node-gyp) to build this module.

Friendly Note
==============

This is my first C++ addon for Node.js 🤗