
## CFmask Based Water Detection Version 1.0.0 - Release Notes

Release Date: March 2016

See git tag [cfwd-version_1.0.0]

This application adds water flags to the Level 2 QA Band for Landsat 4, 5, 7, and 8.

## Product Descriptions
See the [TODO TODO TODO](http://landsat.usgs.gov/CDR_LSR.php) product guide for more information about the Level 2 QA product.

## Release Notes
* Initial implementation
* Algorithm derived from our CFmask product code.

## Installation

### Dependencies
* ESPA raw binary libraries, tools, and it's dependencies, found here [espa-product-formatter](https://github.com/USGS-EROS/espa-product-formatter)

### Environment Variables
* Required for building this software
```
export PREFIX="path_to_Installation_Directory"
export XML2INC="path_to_LIBXML2_include_files"
export XML2LIB="path_to_LIBXML2_libraries_for_linking"
export LZMALIB="path_to_LZMA_libraries_for_linking"
export ZLIBLIB="path_to_ZLIB_libraries_for_linking"
export ESPAINC="path_to_ESPA_PRODUCT_FORMATTER_include_files"
export ESPALIB="path_to_ESPA_PRODUCT_FORMATTER_libraries_for_linking"

```

### Build Steps
* Clone the repository and replace the defaulted version(master) with this
  version of the software
```
git clone https://github.com/USGS-EROS/espa-surface-water-extent.git
cd espa-surface-water-extent
git checkout cfwd-version_<version>
```
* Build and install the application specific software
```
make all-cfwd
make install-cfwd
```

## Usage
See `surface_water_qa.py --help` for command line details.<br>
See `surface_water_qa.py --xml <xml_file> --help` for command line details specific to the Landsat 4, 5, 7, and 8 application.<br>
See `cfmask_water_detection --help` for command line details when the above wrapper script is not called.

### Environment Variables
* PATH - May need to be updated to include the following
  - `$PREFIX/bin`

### Data Processing Requirements
This version of the water detection application requires the input products to be in the ESPA internal file format.

The following input data are required to generate this surface water product:
* Top of Atmosphere

The Top of Atmosphere products can be generated using the software found in our [espa-surface-reflectance](https://github.com/USGS-EROS/espa-surface-reflectance) project.  Or through our ondemand processing system [ESPA](https://espa.cr.usgs.gov), be sure to select the ENVI output format.

This surface water product is <b>NOT</b> currently available in the [ESPA](https://espa.cr.usgs.gov) processing system.

### Data Postprocessing
After compiling the [espa-product-formatter](https://github.com/USGS-EROS/espa-product-formatter) libraries and tools, the `convert_espa_to_gtif` and `convert_espa_to_hdf` command-line tools can be used to convert the ESPA internal file format to HDF or GeoTIFF.  Otherwise the data will remain in the ESPA internal file format, which includes each band in the ENVI file format (i.e. raw binary file with associated ENVI header file) and an overall XML metadata file.

## More Information
This project is provided by the US Geological Survey (USGS) Earth Resources
Observation and Science (EROS) Land Satellite Data Systems (LSDS) Science
Research and Development (LSRD) Project. For questions regarding products
produced by this source code, please contact the Landsat Contact Us page and
specify USGS CDR/ECV in the "Regarding" section.
https://landsat.usgs.gov/contactus.php
