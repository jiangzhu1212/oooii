=== BC6HBC7EncoderDecoder11 ===

BC6H and BC7 are brand new formats, and as such they are poorly documented and have no 3rd-party, or even 1st-party tool support. I mean nothing works! not even this sample as it comes out of the June 2010 SDK!

This code is the addition of a Microsoft-issued patch that makes things work again as well as a reconfigure to make this code a lib that can be linked to rather than a stand-alone exe example.

NOTE: Not even the DirectX Texture Tool that ships with the SDK loads BC6/BC7 encoded files! The only way to confirm things are behaving well is to compress to BC6/7 and then decode it back to a more mainstream format, but as of the writing of this Readme, this code does work as advertised, it's just often hard to prove!