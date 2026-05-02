````markdown
# Enable rpcbind at boot
```sh
sysrc rpcbind_enable=YES
````

# Start it now

```sh
service rpcbind start
```

# Verify it’s listening on UDP and TCP 111

```sh
rpcinfo -p | grep '\(111\)'
```

# Check firewall rules if UDP entry is missing

```sh
# For pf
pfctl -sr | grep 111

# For ipfw
ipfw show | grep 111
```

```
```

