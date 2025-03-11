# How to expand drp-ai memory area
This page explains about the DRP-AI memory expand.

## Overview
1. Extract *.dtb file from SD card.
2. Modify *.dtb file that DRP-AI memory expand.
3. Writeback *.dtb file to SD card.


<center>  <img src=./img/expand_img.png></center>  

+ Modify with Linux PC
 search "DRP-AI" keyword with *.dts and change "reg = <0x02 0x40000000 0x00 0x<span style="color: red; ">2</span>0000000>;" to "reg = <0x02 0x40000000 0x00 0x<span style="color: red; ">8</span>0000000>;"
```
$ apt-get install device-tree-compiler
$ dtc -I dtb -O dts -o r9a09g057h4-evk-ver1.dts r9a09g057h4-evk-ver1.dtb
<edit r9a09g057h4-evk-ver1.dts>
- before--------------------------------
    DRP-AI@240000000 {
        reusable;
        reg = <0x02 0x40000000 0x00 0x20000000>; 
        phandle = <0x5e>;
        };
    };
---------------------------------------
- after--------------------------------
    DRP-AI@240000000 {
        reusable;
        reg = <0x02 0x40000000 0x00 0x80000000>; 
        phandle = <0x5e>;
        };
    };
---------------------------------------
$ dtc -I dts -O dtb -o r9a09g057h4-evk-ver1.dtb r9a09g057h4-evk-ver1.dts
```
