"use strict";

const { Monitor } = require('bindings')('regmon.node');

module.exports = function(hkey, subkey, callback){

  const HKEY = ["HKCR","HKCU","HKLM","HKU","HCC"];

  if ( typeof(hkey) !== 'string' || !hkey ||
       typeof(subkey) !== 'string' || !subkey || 
       typeof(callback) !== 'function' ) 
  {
    throw new Error( "Invalid function arguments" );
  }

  if (!HKEY.includes(hkey)) throw new Error( `Invalid root key! Possible values are: "${HKEY.toString()}"` );

  const monitor = new Monitor(hkey,subkey.replace(/([\/])/g,"\\"));
  
  monitor.watch(callback);

  return { 
    close: function(){
      monitor.close();
    }
  }

}
