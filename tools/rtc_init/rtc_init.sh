#!/bin/sh

sudo picotool load build/rtc_sync_client.uf2 -f

if [ $? -eq 0 ]
then
	echo Success!
fi
