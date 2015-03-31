add_executable(i-n-sniffer
eventprinter.cpp
eventprinter.h
i-n-extractor.cpp
nmaccesspoint.cpp
nmaccesspoint.h
nmactiveconnection.cpp
nmactiveconnection.h
nmconnsettings.cpp
nmconnsettings.h
nmroot.cpp
nmroot.h
nmsettings.cpp
nmsettings.h
nmwirelessdevice.cpp
nmwirelessdevice.h
ofonomodemmodem.cpp
ofonomodemmodem.h
ofonomodemnetworkregistration.cpp
ofonomodemnetworkregistration.h
ofonomodemsimmanager.cpp
ofonomodemsimmanager.h
ofonoroot.cpp
ofonoroot.h
urfkillroot.cpp
urfkillroot.h
urfkillswitch.cpp
urfkillswitch.h
)

qt5_use_modules(i-n-sniffer Core DBus)

install(
  TARGETS i-n-sniffer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
