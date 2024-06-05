<p align="center">
  <img src="https://github.com/PlayingWithFusion/Arduino_SAMD/assets/77081880/05cc5ad2-8b62-4e97-ac00-dcc4c4118d5d" />
</p>

<h1 align="center">Playing With Fusion MAX31865 Arduino Library</h1>

![SEN-30202 ISO](http://www.playingwithfusion.com/include/getimg.php?imgid=1153)
![SEN-30201 ISO](http://www.playingwithfusion.com/include/getimg.php?imgid=1117)

Arduino library designed to interface Arduino-compatible hardware with the Maximum MAX31865 RTD to digital converter IC over SPI, used for digitizing RTD measurements. Examples provided to interface with SEN-30201, FDQ-30002, and SEN-30202.

# Library Documentation

Brief documentation for this library. Make sure to see "examples" folder for ready-to-upload programs.

## Configuration

Configuration of a RTD requires the user to know:

- How many wires were used to wire the RTD

- The type of RTD (PT100 or PT1000)

A basic configuration for a PT100 RTD in 3-Wire configuration could look like this:

```cpp
MAX31865 rtd;

void setup() {
  // Where "2" is the SPI chip select
  rtd.begin(2, RTD_3_WIRE, RTD_TYPE_PT100);
}
```

# Compatible Playing With Fusion Products

- SEN-30201: <a href="http://www.playingwithfusion.com/productview.php?pdid=25">PT100 1-ch</a>, <a href="http://www.playingwithfusion.com/productview.php?pdid=26">PT1000 1-ch</a>

= SEN-30202: <a href="http://www.playingwithfusion.com/productview.php?pdid=29">PT100 2-ch</a>, <a href="http://www.playingwithfusion.com/productview.php?pdid=30">PT1000 2-ch</a>

Questions? Feel free to <a href="http://www.playingwithfusion.com/contactus.php">contact us!</a>
