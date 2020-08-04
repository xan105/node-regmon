"use strict";

const regmon = require("../index.cjs");

console.log("0");
const mon = regmon("HKCU","Software/Microsoft/Windows/CurrentVersion/Run",function(){
  console.log("2");
  mon.close();
});
console.log("1");
