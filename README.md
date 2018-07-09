# SDumper

SDumper (Socker Dumper) listens to /tmp/sdumper.sock (a datagram UNIX socket) and writes everything to a file in its working directory.

The data is compressed and written to the file in gzip format.

The file is rotated (more or less) daily.

This is useful for dumping metrics from telegraf or collectd for long term storage.

