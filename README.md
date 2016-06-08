# LmbrCoherentGT
=====================================
An Amazon Lumberyard gem for integrating CoherentGT

Installation / Integration
==========================
* Paste the folder into the dev/Gems folder
* Enable the gem in the project configurator
* Add the following to AssetProcessorPlatformConfig.ini:
 ```
[RC hlsl]
glob=*.hlsl
params=copy

[RC ihlsl]
glob=*.ihlsl
params=copy
```


