
This is the algorithm description derived from the prototype implementation
provided by the authors and subsequent conversations and emails.


==============================================================================
Algorithm Authors:

    John W. Jones
    Research Geographer
    Eastern Geographic Science Center
    U.S. Geological Survey
    email: jwjones@usgs.gov

    Michael J. Starbuck
    Physical Scientist
    Earth Resources Observation and Science Center
    U.S. Geological Survey
    email: mstarbuck@usgs.gov


==============================================================================
Algorithm Description - Overview:

    The algorithm relies on a series of relatively simple and efficient water
    detection tests, each with their own output code for a "positive" test
    result or 0 for a negative test result.  Resulting in a 5 digit output
    value in the range 00000 to 11111, where each digit correspondes to a
    specific test.

    These test results are then further refined(recoded) to the following
    values:

          0 -> Not Water
          1 -> Water - High Confidence
          2 -> Water - Moderate Confidence
          3 -> Partial Surface Water Pixel
          9 -> Cloud or Cloud Shadow
        255 -> Fill (no data)

    The algorithm provides an output of 3 bands.  The first band represents
    the Raw DSWE (recoded values 0, 1, 2, 3, and 255).  The second band
    represents the Raw DSWE with filtering applied for Cloud and Cloud Shadow
    (recoded values 0, 1, 2, 3, 9, and 255).  The third band represents the
    Raw DSWE with filtering applied for Percent-Slope, Cloud and Cloud Shadow
    (recoded values 0, 1, 2, 3, 9, and 255).

    Percent-Slope is utilized to remove locations where the terrain is too
    sloped to hold water.  Any values that meet this criteria are recoded to
    a value of 0.


==============================================================================
Algorithm Description - Inputs:

    Primary source of the input is Surface Reflectance derived from L1T
    products.  Specifically the Blue, Green, Red, NIR, SWIR1, and SWIR2
    Surface Reflectance bands, along with the CFMASK band.

    A DEM is also utilized to generate an internal Percent-Slope band for the
    required slope filtering.


==============================================================================
Algorithm Description - Detailed:


    NOTE: Keep in mind during the processing of the Raw DSWE band, the output
          is filtered for fill data and those values are set to 255.


    Raw DSWE -> Output:

         1) Calculate Modified Normalized Difference Wetness Index (MNDWI)
            from the Green and SWIR1 bands.

            mndwi = (Green - SWIR1) / (Green + SWIR1)

         2) Calculate Multi-band Spectral Relationship Visible (MBSRV) from
            the Green and Red bands.

            mbsrv = Green + Red

         3) Calculate Multi-band Spectral Relationship Near-Infrared (MBSRN)
            from the NIR and SWIR1 bands.

            mbsrn = NIR + SWIR1

         4) Calculate Automated Water Extent Shadow (AWEsh) from the Blue,
            Green, and SWIR2 bands, along with MBSRN.

            awesh = (Blue
                     + (2.5 * Green)
                     + (-1.5 * mbsrn)
                     + (-0.25 * SWIR2))

         5) Perform the first test by comparing the MNDWI to a Wetness Index
            threshold; Where the threshold ranges from 0.0 to 2.0 and is
            defaulted to a value of 0.0123.

            if (mndwi > 0.0123) set the ones digit  (Example 00001)

         6) Perform the second test by comparing the MBSRV and MBSRN values to
            each other.

            if (mbsrv > mbsrn) set the tens digit  (Example 00010)

         7) Perform the third test by comparing AWEsh to a Automated Water
            Extent Shadow threshold; Where the threshold ranges from -2.0 to
            2.0 and is defaulted to a value of 0.0.

            if (awesh > 0.0) set the hundreds digit  (Example 00100)

         8) Perform the fourth test by comparing the MNDWI along with the
            NIR and SWIR1 bands to the following thresholds. Partial Surface
            Water Test-1 threshold; Where the threshold ranges from -2.0 to
            2.0 and is defaulted to a value of -0.5.  Partial Surface Water
            Test-1 NIR threshold; Where the threshold ranges from 0 to data
            maximum and is defaulted to a value of 1500.  Partial Surface
            Water Test-1 SWIR1 threshold; Where the threshold ranges from 0 to
            data maximum and is defaulted to a value of 1000.

            if (mndwi > -0.5
                && SWIR1 < 1000
                && NIR < 1500) set the thousands digit  (Example 01000)

         9) Perform the fourth test by comparing the MNDWI along with the
            NIR and SWIR2 bands to the following thresholds. Partial Surface
            Water Test-2 threshold; Where the threshold ranges from -2.0 to
            2.0 and is defaulted to a value of -0.5.  Partial Surface Water
            Test-2 NIR threshold; Where the threshold ranges from 0 to data
            maximum and is defaulted to a value of 1700.  Partial Surface
            Water Test-2 SWIR2 threshold; Where the threshold ranges from 0 to
            data maximum and is defaulted to a value of 650.

            if (mndwi > -0.5
                && SWIR2 < 1000
                && NIR < 2000) set the ten-thousands digit  (Example 10000)

        10) Recode the results from the previous steps using the following
            ranges and values.

            11001 11111 : 1 (Water - High Confidence)
            10111 10999 : 1
            01111 01111 : 1

            11000 11000 : 3 (Partial Surface Water Pixel)
            10000 10000 : 3
            01000 01000 : 3

            10012 10110 : 2 (Water - Moderate Confidence)
            10011 10011 : 2
            10001 10010 : 2
            01001 01110 : 2
            00010 00111 : 2

            00000 00009 : 0 (Not Water)

        11) Output the Raw DSWE


    Raw DSWE -> Cloud or Cloud Shadow-> Output:

         1) Perform a test by comparing CFMASK band to the Cloud and Cloud
            Shadow values.

            if (cfmask == 2 or cfmask == 4) set the cloud filtered Raw DSWE to
                a recoded value of 9, otherwise set to Raw DSWE

         2) Output the Cloud and Cloud Shadow filtered Raw DSWE.


    Raw DSWE -> Percent-Slope -> Cloud or Cloud Shadow -> Output:

         1) Build a Percent-Slope band from the DEM source.

         2) Perform a test by comparing the Percent-Slope band to a
            Percent-Slope threshold; where the threshold ranges from 0.0 to
            100.0 and is defaulted to a value of 9.0.

            if (percent-slope >= 9.0) set the Percent-Slope filtered Raw DSWE
                to a recoded value of 0, otherwise set to Raw DSWE

         3) Perform a test by comparing CFMASK band to the Cloud and Cloud
            Shadow values.

            if (cfmask == 2 or cfmask == 4) set the Percent-Slope filtered
                Raw DSWE to a recoded value of 9, otherwise leave alone

         4) Output the Percent-Slope, Cloud and Cloud Shadow filtered Raw DSWE.

