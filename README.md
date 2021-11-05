# Kea Hook Curl Options

A Kea DHCP hook to support external http requests. Responce from a web server
should be json list like
```
{
  224: "some string",
  225: "other string"
}
```

Only strings are now supported. Json keys are used as coresponding option numbers
that will be used to create/redefine options by DHCP server answer inside dhcp packet.
