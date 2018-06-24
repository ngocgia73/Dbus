/* how to run this example */

===step 1: need adjust the config file===
+ cd /etc/dbus-1/system.d/

===step2: open file avahi-dbus.conf use vim or any editor you want===
+ vi avahi-dbus.conf
===step3: add content below into this file config===

<policy user="giann">
  <allow own="com.giann.NameOfProcess"/> 
</policy>
<policy user="root">
  <allow own="com.giann.NameOfProcess"/> 
</policy>

NOTE : "com.giann.NameOfProcess" is name of connection in dbus
