dbus-monitor --session \
    "type=signal,
     sender='com.ubuntu.connectivity1',
     path=/com/ubuntu/connectivity1/Private,
     interface=org.freedesktop.DBus.Properties,
     member=PropertiesChanged"

