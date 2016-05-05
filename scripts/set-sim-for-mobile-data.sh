dbus-send --session --print-reply           \
    --dest=com.ubuntu.connectivity1         \
    /com/ubuntu/connectivity1/Private       \
    org.freedesktop.DBus.Properties.Set     \
    string:com.ubuntu.connectivity1.Private \
    string:SimForMobileData                 \
    variant:objpath:$1
sh get-sim-for-mobile-data.sh
