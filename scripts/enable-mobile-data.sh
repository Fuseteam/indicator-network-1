dbus-send --session --print-reply           \
    --dest=com.ubuntu.connectivity1         \
    /com/ubuntu/connectivity1/Private       \
    org.freedesktop.DBus.Properties.Set     \
    string:com.ubuntu.connectivity1.Private \
    string:MobileDataEnabled                \
    variant:boolean:true
sh get-mobile-data-enabled.sh
