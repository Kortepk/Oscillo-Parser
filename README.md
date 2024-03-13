Virtual desktop parser for view oscillographe data from com port. 
Create in QT

**Number of channels:** add or reduces oscilloscope screen/graphs

**Group size:** change changes the size of the graphs

# Data Format
**Int32:** A string consisting of this format should be sent to the com port input:
<img src="img/int32.png" height = "100">

**Float:** A string consisting of this format should be sent to the com port input:
<img src="img/float.png" height = "100">

**5 bytes:** The bottom line is that you need to transfer raw data, namely split int32 into 4 8-bit numbers and then send them to com. To separate the channels, we send a number with the first byte - the defining channel number.
<img src="img/5 bytes.png" height = "150">

Tpd = 1 secons
