dbus-send --session --print-reply           \
    --dest=com.ubuntu.connectivity1         \
    /com/ubuntu/connectivity1/Private       \
    org.freedesktop.DBus.Properties.Get     \
    string:com.ubuntu.connectivity1.Private \
    string:Modems
