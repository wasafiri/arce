# arce
Example code for using a Particle Photon to interface with an intercom and send SMS alert + do stuff with Philips HUE lights when someone buzzes.

To use, copy the .ino code and paste into the Particle cloud (build.particle.io) and flash onto your device.

Inspiration taken from https://gist.github.com/jamesabruce/8bc9faf3c06e30c3f6fc

Examples for creating webhooks to send SMS using either Twilio or Plivo are included here. Plivo is quite a bit cheaper for outgoing SMS and free for incoming SMS.

To create a webhook, install the Particle CLI (spark/particle-cli), navigate to the directory with the json file. From there, creating a webhook is as easy as:

`particle webhook create twilio.json`
