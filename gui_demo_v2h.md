---
layout: tutorial
title: How to Use Guide
permalink: /gui_demo_v2h.html
---
# 1. Overview

This package is a demo system which can confirm the performances of AI models on RZ/V2H Evaluation Board Kit.

Users can select the AI model applications on the GUI menu by using the USB mouse, without typing any command.

This package supports the following AI models.

* Depth Estimation
  * MiDaS (Dense model only)
* Semantic Segmentation
  * TopFormer-Tiny (Dense model only)
* Object Detection
  * YOLOX-S (Dense model and 70% sparse model)
  * YOLOv2 (Dense model and 90% sparse model)
* Pose Estimation
  * HRNet (Dense model and 90% sparse model)
* Classification
  * ResNet50 (Dense model and 90% sparse model)

Sparse models are created by the pruning tool in DRP-AI extension package, and realize faster performance and lower foot print than Dense model.

# 2. Package Contents

<table>
 <tr>
  <th colspan="2">Contents</th>
  <th>File name</th>
 </tr>
 <tr>
  <td rowspan="2">SD card image file (WIC format)</td>
  <td>USB Camera</td>
  <td>rzv2h_ai_performance_demo_usb_camera_sd_image.wic.gz</td>
</tr>
<tr>
  <td>MIPI Camera</td>
  <td>rzv2h_ai_performance_demo_mipi_camera_sd_image.wic.gz</td>
</tr>
<tr>
  <td colspan="2">License files *</td>
  <td>licenses.zip</td>
</tr>
</table>

>Note: Licence files for the software that composes the above SD card image.

# 3. Required Hardware

## 3.1 Target board and peripheral equipment

| Item                                      | Note                                                         |
| ----------------------------------------- | ------------------------------------------------------------ |
| RZ/V2H Evaluation Board Kit               | Hereinafter referred to as the "RZ/V2H EVK"                  |
| USB PD (100W) + USB Type-C Cable          |                                                              |
| micro SD Card                             | **Required 8GB or more of the capacity.**<br />This package is tested by using SDHC microSD 16GB. |
| MIPI Camera*                              | e-con Systems e-CAM22_CURZH
| USB Camera*                               | Supported resolution: 640x480<br /> Supported format: 'YUYV' (YUYV 4:2:2) |
| USB Hub                                   | Only when using a USB camera.                                |
| HDMI Monitor + HDMI Cable                 |                                                              |
| USB Mouse                                 |                                                              |

*Select MIPI or USB Camera according to the SD Card Image to be used.

## 3.2 PC for writing SD Card image

| Item                 | Note                                                         |
| -------------------- | ------------------------------------------------------------ |
| Linux PC             | This package is tested on the Linux PC with Ubuntu OS 20.04. |
| micro SD card reader | Required if the Linux PC does NOT equip any micro SD card reader. |

# 4. How to use this package

## 4.1. How to write the SD Card Image into the SD Card

1. Copy the following files into any directory on the Linux PC.
   * `rzv2h_ai_performance_demo_***_camera_sd_image.wic.gz`
2. Execute the following command to install `bmap-tools` into the Linux PC. (If already installed, this step can be skipped.)
{% highlight console %}
$ sudo apt install bmap-tools
{% endhighlight %}
{:start="3"}
3. Insert the micro SD Card into the micro SD Card reader, and connect the micro SD Card reader to the Linux PC.
4. Execute `lsblk` command, and confirm the device file name of the micro SD Card and the size of it.
   * In this example below, the device file name of the micro SD Card is `sda` listed on the  `NAME` column.
{% highlight console %}
$ lsblk
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
:
sda           8:0    1  14.2G  0 disk
├─sda1        8:1    1  16.9M  0 part
└─sda2        8:2    1     4G  0 part /mnt/sd
:
{% endhighlight %}
* If any partition of the micro SD card is mounted on the Linux PC filesystem, please unmount all partition.
  * In the example above, the `sda2` partition of the micro SD card is mounted on `/mnt/sd` on the Linux PC file system. You can unmount the partition by executing the following command.
{% highlight console %}
$ sudo umount /dev/sda2
or
$ sudo umount /mnt/sd
{% endhighlight %}

{:start="5"}
5. Write the SD Card image file (`rzv2h_ai_performance_demo_***_camera_sd_image.wic.gz`) into the micro SD card by the following command.
    * The command below writes the SD Card image to the `sda` device file.
    Please replace `sda` with a device file name displayed on your Linux PC by  `lsblk` command.

{% highlight console %}
$ sudo bmaptool copy --nobmap rzv2h_ai_performance_demo_***_camera_sd_image.wic.gz /dev/sda
{% endhighlight %}

> **Note: It will take approximately 5 minutes to write the image file to the SD card.**

## 4.2 How to configure RZ/V2H EVK

First, please prepare RZ/V2H EVK, on which the EXP Board is mounted. Please refer to [RZ/V2H Evaluation Board Kit Hardware Manual](https://www.renesas.com/us/en/document/mah/rzv2h-evaluation-board-kit-secure-type-hardware-manual?r=25471796) for more details.

Next, please configure the RZ/V2H EVK and connect the peripheral equipment to it in the order according to the figure below.

![rzv2h_evk_demo_setting](img/rzv2h_evk_demo_setting.png)

1. Insert the micro SD Card to the RZ/V2H EVK.
2. Configure the DSW1 and DSW2 according the figure above.
3. Connect the USB Mouse (and USB camera via USB hub if using a USB camera) to the USB 2.0 terminal
on RZ/V2H EVK.
4. Connect the HDMI monitor to the RZ/V2H EVK with HDMI cable.
5. If using a MIPI camera, connect the MIPI camera to the RZ/V2H EVK.
    * Please connect to CN7 terminal.
    * Regarding how to connect the MIPI camera, please refer to the following link.
        * [connect_e-cam22_curzh_to_rzv2h_evk.png](https://github.com/renesas-rz/rzv_drp-ai_tvm/blob/main/how-to/img/connect_e-cam22_curzh_to_rzv2h_evk.png)
6. Connect the USB PD 100W to the RZ/V2H EVK with USB Type-C cable.
7. Turn the SW3 to ON.
8. Turn the SW2 to ON to power on the RZ/V2H EVK.

Finally, after waiting for about 10 seconds, GUI menu will be displayed on the HDMI monitor.

## 4.3 How to start the AI applications on the GUI menu

Users can select and start the AI model applications on the GUI menu by using the USB mouse without typing command.

1. Click on the [Start] button of any AI model application to run it, then the AI model application result will be displayed.
   * The processing times of AI model the user selected will be displayed at the upper right corner.

2. If user clicks to the blue (x) icon at the lower right corner, the application will finish. 
   * The GUI menu will be displayed again, and user can will select the next AI model application on the GUI menu.

## 4.4 How to shut down the GUI menu and the RZ/V2H EVK

1. Clicks to the [x] icon at the upper right corner on the GUI menu, then the software will shut down.

2. On the screen, check that shutdown procedure runs and the HDMI display is blacked out.

3. Turn the SW2 to OFF.

4. Turn the SW3 to OFF.

# 5. Version History

* v1.00 issued on 26th June 2024. 
  * First version based on DRP-AI TVM 2.3.1.
* v1.10 issued on 29th November 2024.
  * Update for DRP-AI TVM v2.4.1.
* v1.20 issued on 6th December 2024
  * Update for USB Camera version.
* v1.30 issued on 11th March 2025
  * Update for DRP-AI TVM v2.5.0.
