The original build of this master clock controller is described in the
January 2015 issue of Nuts and Volts:

   http://nutsvolts.texterity.com/nutsvolts/201501/?folio=32&pg=32#pg32

Note: You may need a subscription to use the link.

The 5/24 VDC power supply is a straightforward rectifier/capacitor/regular
design attached to a transformer from an IBM Proprinter. This has a couple of
issues:

- You may have or be able to find the necessary transformer.

- This sort of power supply is quite inefficient and draws a lot of power even
  when nothing is going on, which for a clock like this is most of the time.
  
I therefore recommend using a switched power supply instead. The following
switched power supply does the job nicely for me and (currently) costs $24.95:

   http://www.mouser.com/Search/ProductDetail.aspx?R=RPD-65Dvirtualkey63430000virtualkey709-RPD65D
    
I also had issues with the IRFZ44N HEXFET used in the design. I eventually
switched to a standard design based on the FQP27P06 MOSFET, which is as
common as dirt. Note that you can also buy a prebuilt driver board, e.g.:

   https://www.sparkfun.com/products/10618

   http://tronixlabs.com/arduino/shields/motor/freetronics-n-drive-mosfet-shield-for-arduino-australia/ 

Or you can build a kit:

   https://www.tindie.com/products/MakersBox/mosfet-jr-arduino-shield/
   
There are so many options available and the prices are sufficiently low that
I really can't recommend the breadboard approach.
