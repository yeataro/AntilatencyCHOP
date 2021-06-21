# AntilatencyCHOP

[Antilatency]:https://antilatency.com/ "HomePage"
[TouchDesigner]:https://derivative.ca/ "HomePage"

[TouchDesigner] C++ CHOP for [Antilatency].

![](img/sc01.png)

## Warning
Early test version, not suitable for production environment. Anyone who is interested in this feature is welcome to improve it.


## To-do
- [ ] Support more than one ALT. 
- [ ] Multi-threaded sampling at high speed.
- [x] Export the mark points of the environment configuration to info DAT.
    - [ ] keep info DAT for device info, create an py object for the mark points.
- [ ] [SDK 3.0.0](https://github.com/AntilatencySDK/Release_3.0.0/tree/subset-1a9364188c87e306d2992003ab6d2f5d1b1333b7)
- Paramaters:
    - [ ] Add "Tag" to the channel name. (Toggle)
    - [ ] Add "Serial number" to the channel name. (Toggle)
    - [ ] Add "Device name" to the channel name. (Toggle)
    - [ ] Output RAW pose. (Toggle)
    - [ ] Max ALTs. (Int)

## Known issues
- ~~There are more than two operators at the same time will crash. (Even in different processes)~~ 
    >Fix - but deviceNetwork won't setup again.

- ~~Some specific environment code will cause a crash (it is guessed that it can be decoded by bese64 but the content is incorrect).~~
    >Fix.

## Disclaimer
- I am not responsible for any risks caused by using this product in any production site.
- All SDKs and quoted samples are owned by their sources, and I do not claim any rights regarding them.

