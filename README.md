# Remote-Logging-Module

An onboard data logging system for our cars, using radio transmission for remote data monitoring.

## Build + Run

For the transmitter module:

`pio run -e transmitter -t upload && pio device monitor -e transmitter`

For the receiver module:

`pio run -e receiver -t upload && pio device monitor -e receiver`
