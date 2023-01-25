# A1AndroidControlFix
'emulates' a controller using data received from an Arcade1up joystick/button encoder on android based platforms, specifically designed for and tested on Simpsons, TMNT (In Time) and "Fighter Droids".

# Cabinet Pre-requisites (Cabinet Modifications)
This appplication requires access to `/dev/ttySXX` where XX is the specific encoder attached to the serial interface of the chipset used on the cabinet, this is achieved by running as a system application as existing SELinux policy is configured to allow system applications to access this device. Additionally however, we need access to `/dev/uinput` and the ability to write to it in order to 'emulate' a new controller. On the cabinets which we currently support (Simpsons, "Fighter Droids") this is achieved by reflashing the PCB to a image with the SELinux policies modified ahead of time.

The issue is that by default the SELinux policy on the system does not allow for system based applications to access/write to `/dev/uinput` we can overcome this using other tricks that we won't disclose to bypass SELinux and abuse Android security overall but the better way to apporach this is to modify the existing SELinux policy to add the support.

## SELinux Modifications
The below is an example of the modifications performed to the Simpsons cabinet firmware in order to achieve the necessary permissions.

#### Parition: system
* /etc/selinux/plat_sepolicy.cil 

We added the following line to the policy `(allow system_app uhid_device (chr_file (ioctl read write getattr lock append map open)))` this will allow the system application to read,write,and perform additional operations against anything already deemed as a "uhid_device" on the system/previously in SELinux policy.

In order to educate others on how this SELinux policy works we'll break down each piece of the rule below:
* `allow` - Telling the operation to be allowed overall.

* `system_app` - This is not a 'default' or known thing to SELinux, this is a domain which is defined in plat_seapp_contexts, plat_seapp_contexts are assigned to applications based on keys, in this case we use a 'platform' key and the Android OS associates this context to our application based on that key which places us in the domain 'system_app' the entry for the system_app context can be seen here `user=system seinfo=platform domain=system_app type=system_app_data_file` zygote is what handles the overall assignment of domains based on keys in the Android security model.

* `uhid_device` - This is a 'file' which is defined in plat_file_contexts of the same directory viewing the entry for it you can see that `/udev/input` was assigned to the object `uhid_device` the entry can be seen here: `/dev/uinput		u:object_r:uhid_device:s0`

Additionally, some other domains have access to uhid devices as well:
```
(allow hal_bluetooth uhid_device (chr_file (ioctl read write getattr lock append map open)))
(allow virtual_touchpad uhid_device (chr_file (ioctl write lock append map open)))
(allow bluetooth uhid_device (chr_file (ioctl read write getattr lock append map open)))
(allow shell uhid_device (chr_file (ioctl read write getattr lock append map open)))
```

The reason we're mentioning this is that you could potentially run your application under one of these other domains in order to get the permission to access the device, but then you'd lose access to reading the serial data... in theory you could overcome this by running two applications or daemons where one transmits the controls to the other. Overall, this was deemed to be an inefficient solution to the problem but worth noting.

* `(chr_file (ioctl read write getattr lock append map open)))`

This portion of the rule is mostly self explanatory, and we won't go into additional detail outside of the fact that the data after chr_file is the permissions being granted.

#### Note: Pre-compiled SELinux policy on Android
In doing research on how SELinux policies are compiled, you'll note that there are pre-compiled policies that require re-building the entire OS to get the system to recognize. Yet, the plain-text exists so what is the solution?

Well, Android will fallback to the text based SELinux policy if the hash in the same folder noted by `.sha256` doesn't match the compiled file, see here:
https://android.googlesource.com/platform/system/core/+/android-o-iot-preview-5/init/selinux.cpp#204

So, there's a few ways to approach this one way we've achieved it is to simply remove the data in the `.sha256` file in order to force the system to fail and read the plain-text verison we've now modified.

## dm-verity and avb (android verified boot) - boot loader patches
#### Note: This is not required for every cabinet and was observed primarily on Xmen vs SF and MVC.

On some of the cabinets, dm-verity was enabled and modification of the system or vendor partitions would cause the bootloader to fail. As far as we've seen there's no easy way to overcome this and most publicly documented methods failed to show promising results. So, we came up with our own solution and it's dirty... (You could theoretically use magisk, gain root and avoid most of this but we don't want to install/preload additional unneeded software on cabinets).

The key is in fs_mgr_verify.cpp of the platform_system_core which we're referring to as the bootloader but it's actually the system `init`, when a device is 'unlocked' the system will ignore dm-verity settings or existing metadata and boot normally even if signature verification fails.

This can be observed in the following links:

[fs_mgr_is_device_unlocked](https://cs.android.com/android/platform/superproject/+/master:system/core/fs_mgr/fs_mgr.cpp;drc=e81be34c85590c1f35059e3310dbbf3d6e3e0cf8;bpv=1;bpt=1;l=775)

[read verity metdata (bypass when unlocked)](https://android.googlesource.com/platform/system/core/+/d1fe3bdbd6bcdc7f268f045e6b3b77de4d837a21/fs_mgr/fs_mgr_verity.cpp#785)

[verify_verify_signature (bypass when unlocked)](https://review.blissroms.org/plugins/gitiles/platform_system_core/+/c53d794ca8693630a03ecc54766ffa5ef03b168f/fs_mgr/fs_mgr_verity.cpp#445) - note this is specific to older versions and wasn't observed on the current master for android and was initially discovered through reverse engineering.

With the above knowledge, and knowing that the cabinets use a known/existing android verified boot key. You can simply modify the `init` binary in order to bypass the dm-verity checks by forcing the device to believe it's 'always' unlocked or make the checks never fail.

This is what we've done, and re-packaged/reflashed the boot to the system in order to bpyass the above.

## Enabling adb at boot time
Yet another hack we introduced was manipulating the `init.rc` script inside of the boot and system partitions in order to enable adb without having to go into developer options, we did this so that after the system is loaded we can manipulate further using adb in a running system (disabling the current input driver, installing ours, enabling the installation of retroarch or preloading of other applications).

While this isn't entirely necessary, we felt it was the best way to have the ability to maintain a stock system or later enable the ability to uninstall by not 'baking' software into the firmware that was being reflashed to the underlying system.

We won't go into detail on the modifications made, but should you ever inspect the `init.rc` comparing it against the stock one dumped from the cabinet should give you an idea of what changes were made to force adb to be enabled from boot time.


## Flashing the above to the cabinet after modifications
#### Note: This isn't recommended but we're documenting it anyway.

There are known vendor tools made for the SoC inside of the cabinets which allow you to read/write to data that exists on the SBC's storage, these tools are utilized by our 'loader' / 'installation' software in order to reflash this newly created boot.img and system.img and overwrite the existing data. This of course means you must have a dump of the system first which thanks to the Android escapes everyone has used is quite easy to get ahold of and make a backup.



## Building the Control Fix
TBD

## Testing the Control Fix
Testing mostly consists of the usage of `logcat` to see outputs, as well as examining data being received and how the underlying cabinet is interacting to this. Beyond that to uninstall and reinstall the control fix on a running system where the pre-requisites above have been met you can do the following in a command prompt in the `platform-tools` folder provided by google:

```
adb shell pm uninstall com.android.SimpsonsControllerFix
adb install -g ControlFix.apk
adb shell am start com.android.SimpsonsControllerFix/.MainActivity
adb shell sync
wait 10 seconds
adb reboot
```

## Supporting 2 Player configuration vs 4 Player configuration (Simpsons/TMNT vs Fighter Droids)
To support 2 player layouts, simple uncomment `#define TYPE_2PLAYER` before building. Likewise, for 4 player simply uncomment `#define TYPE_4PLAYER`... it's unwise to leave both of these uncommented at the same time.

## Trackball Suppport
Some rudementary trackball support is included in the uploaded code, however... it's not great and it simply attempts to utilize `uinput` to simulate a mouse. Should you want to improve the code feel free to open a PR but at the moment it's kinda stuck where it is.

Look at the `#define TRACK_BALL` area to determine how to enable the code when building.

## Adapting support to more android cabinets
This shouldn't be necessary (on standard joystick cabinets); MVC2, Shinku and Yoga Flame use the same control encoder, however keep in mind the cabinet pre-requisites documented above.

Anything with unique controls such as Lightguns or Steering Wheel (Ride Racer) would have to be adapted separately. The idea/goal for Ridge Racer was to use `uinput` to simulate an existing steering wheel device or joystick of a controller vs the d-pad mechanism/implementation used on the system currently by the 'robot' which is simulated at a kernel level.

## Additional notes
This is not an android application and we do not load the native application as a shared library, we package it as one and then independetly run a C binary on the android system. This was done for a couple of reasons including android killing the parent application when in the background for a period of time, there are a lot of tricks/hacks like this used to make this possible.


