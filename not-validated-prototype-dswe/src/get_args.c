
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <getopt.h>
#include <error.h>
#include <string.h>
#include <limits.h>


#include "espa_metadata.h"
#include "parse_metadata.h"


#include "const.h"
#include "dswe.h"
#include "utilities.h"
#include "get_args.h"


/* Specify default parameter values */
/* L4-7 defaults */
static float wigt_l47_default = 0.0123;
static float awgt_l47_default = 0.0;
static float pswt_1_l47_default = -0.5;
static float pswt_2_l47_default = -0.5;
static int pswst_1_l47_default = 1000;
static int pswnt_1_l47_default = 1500;
static int pswst_2_l47_default = 1000;
static int pswnt_2_l47_default = 2000;
/* L8 defaults */
static float wigt_l8_default = 0.1163;
static float awgt_l8_default = 0.0;
static float pswt_1_l8_default = -0.67;
static float pswt_2_l8_default = -0.67;
static int pswst_1_l8_default = 1000;
static int pswnt_1_l8_default = 1500;
static int pswst_2_l8_default = 1000;
static int pswnt_2_l8_default = 2000;

static float percent_slope_default = 6.0;

/* Parameter values should never be this, so use it to determine if a
   parameter was specified or not on the command line before applying the
   default value */
#define NOT_SET -9999.0


/*****************************************************************************
  NAME:  version

  PURPOSE:  Prints the version information for this application.

  RETURN VALUE:  Type = None
*****************************************************************************/
void
version ()
{
    printf ("%s version %s\n", DSWE_APP_NAME, DSWE_VERSION);
}


/*****************************************************************************
  NAME:  usage

  PURPOSE:  Displays the help/usage to the terminal.

  RETURN VALUE:  Type = None
*****************************************************************************/
void
usage ()
{
    version();

    printf ("Dynamic Surface Water Extent\n"
            "Determines and builds Dynamic Surface Water Extent output bands"
            " from Surface\n"
            "Reflectance input data in ESPA raw binary format.\n\n");
    printf ("usage: dswe"
            " --xml <input_xml_filename> [--help]\n\n");
    printf ("where the following parameters are required:\n");
    printf ("    --xml: Name of the input XML file which contains the surface"
            " reflectance,\n"
            "           and top of atmos files output from LEDAPS in raw"
            " binary\n"
            "           (envi) format\n");

    printf ("where the following parameters are optional:\n");
    printf ("    --wigt: Modified Normalized Difference Wetness Index"
            " Threshold\n"
            "            between 0.00 and 2.00\n"
            "            (Defaults - L4-7 %0.3f, L8 %0.3f)\n",
            wigt_l47_default, wigt_l8_default);
    printf ("    --awgt: Automated Water Extent Shadow"
            " Threshold\n"
            "            between -2.00 and 2.00\n"
            "            (Defaults - L4-7 %0.3f, L8 %0.3f)\n",
            awgt_l47_default, awgt_l8_default);

    printf ("    --pswt_1: Partial Surface Water Test-1 Threshold\n"
            "              between -2.00 and 2.00\n"
            "              (Defaults - L4-7 %0.3f, L8 %0.3f)\n",
            pswt_1_l47_default, pswt_1_l8_default);
    printf ("    --pswt_2: Partial Surface Water Test-2 Threshold\n"
            "              between -2.00 and 2.00\n"
            "              (Defaults - L4-7 %0.3f, L8 %0.3f)\n",
            pswt_2_l47_default, pswt_2_l8_default);

    printf ("    --pswnt_1: Partial Surface Water Test-1 NIR Threshold\n"
            "               between 0 and data maximum\n"
            "               (Defaults - L4-7 %d, L8 %d)\n",
            pswnt_1_l47_default, pswnt_1_l8_default);
    printf ("    --pswnt_2: Partial Surface Water Test-2 NIR Threshold\n"
            "               between 0 and data maximum\n"
            "               (Defaults - L4-7 %d, L8 %d)\n",
            pswnt_2_l47_default, pswnt_2_l8_default);

    printf ("    --pswst_1: Partial Surface Water Test-1 SWIR1 Threshold\n"
            "               between 0 and data maximum\n"
            "               (Defaults - L4-7 %d, L8 %d)\n",
            pswst_1_l47_default, pswst_1_l8_default);
    printf ("    --pswst_2: Partial Surface Water Test-2 SWIR2 Threshold\n"
            "               between 0 and data maximum\n"
            "               (Defaults - L4-7 %d, L8 %d)\n",
            pswst_2_l47_default, pswst_2_l8_default);

    printf ("    --percent-slope: Threshold between 0.00 and 100.00"
            " (default value is %0.1f)\n", percent_slope_default);

    printf ("    --use_zeven_thorne: Should Zevenbergen&Thorne's slope"
            " algorithm be used?\n"
            "                        (default is false, meaning Horn's slope"
            " algorithm will\n"
            "                        be used)\n"
            "                        Output using zeven_thorne has *NOT* been"
            " validated.\n");

    printf ("    --use-toa: Should Top of Atmosphere be used instead of"
            " Surface Reflectance\n"
            "               (default is false, meaning Surface Reflectance"
            " will be used)\n"
            "               Also default parameters are taylored to Surface"
            " Reflectance.\n");

    printf ("    --verbose: Should intermediate messages be printed? (default"
            " is false)\n\n");

    printf ("    --help: Will print this usage statement\n\n");
    printf ("Example: dswe"
            " --xml LE70760172000175AGS00.xml\n");
}


/*****************************************************************************
  NAME:  get_args

  PURPOSE:  Gets the command line arguments and validates that the required
            arguments were specified.

  RETURN VALUE:  Type = int
      Value    Description
      -------  ---------------------------------------------------------------
      ERROR    Error getting the command line arguments or a command line
               argument and associated value were not specified.
      SUCCESS  No errors encountered.
*****************************************************************************/
int
get_args
(
    int argc,                    /* I: number of cmd-line args */
    char *argv[],                /* I: string of cmd-line args */
    char **xml_filename,         /* O: input XML filename */
    Espa_internal_meta_t *xml_metadata, /* O: input metadata */
    bool *use_zeven_thorne_flag, /* O: use zeven thorne */
    bool *use_toa_flag,          /* O: process using TOA */
    bool *include_tests_flag,    /* O: include raw DSWE with output */
    bool *include_ps_flag,       /* O: include ps with output */
    float *wigt,                 /* O: tolerance value */
    float *awgt,                 /* O: tolerance value */
    float *pswt_1,               /* O: tolerance value */
    float *pswt_2,               /* O: tolerance value */
    float *percent_slope,        /* O: slope tolerance */
    int *pswnt_1,                /* O: tolerance value */
    int *pswnt_2,                /* O: tolerance value */
    int *pswst_1,                /* O: tolerance value */
    int *pswst_2,                /* O: tolerance value */
    bool * verbose_flag          /* O: verbose messaging */
)
{
    int c;
    int option_index;
    char msg[256];
    int tmp_zeven_thorne_flag = false;
    int tmp_toa_flag = false;
    int tmp_verbose_flag = false;
    int tmp_include_tests_flag = false;
    int tmp_include_ps_flag = false;

    struct option long_options[] = {
        /* These options set a flag */
        {"use-zeven-thorne", no_argument, &tmp_zeven_thorne_flag, true},
        {"use-toa", no_argument, &tmp_toa_flag, true},

        {"include-tests", no_argument, &tmp_include_tests_flag, true},
        {"include-ps", no_argument, &tmp_include_ps_flag, true},

        /* These options provide values */
        {"xml", required_argument, 0, 'x'},

        {"wigt", required_argument, 0, 'w'},
        {"awgt", required_argument, 0, 'a'},

        {"pswt_1", required_argument, 0, 'p'},
        {"pswt_2", required_argument, 0, 'q'},

        {"pswnt_1", required_argument, 0, 'n'},
        {"pswnt_2", required_argument, 0, 'o'},

        {"pswst_1", required_argument, 0, 'r'},
        {"pswst_2", required_argument, 0, 's'},

        {"percent-slope", required_argument, 0, 't'},

        /* Special options */
        {"verbose", no_argument, &tmp_verbose_flag, true},
        {"version", no_argument, 0, 'v'},

        /* The help option */
        {"help", no_argument, 0, 'h'},

        /* The option termination set */
        {0, 0, 0, 0}
    };

    if (argc == 1)
    {
        ERROR_MESSAGE ("Missing required command line arguments\n\n",
                       MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Initialize to the not set values */
    *wigt = NOT_SET;
    *awgt = NOT_SET;
    *pswt_1 = NOT_SET;
    *pswt_2 = NOT_SET;
    *percent_slope = NOT_SET;
    *pswnt_1 = NOT_SET;
    *pswnt_2 = NOT_SET;
    *pswst_1 = NOT_SET;
    *pswst_2 = NOT_SET;

    /* loop through all the cmd-line options */
    opterr = 0; /* turn off getopt_long error msgs as we'll print our own */
    while (1)
    {
        c = getopt_long (argc, argv, "", long_options, &option_index);
        if (c == -1)
        {
            /* out of cmd-line options */
            break;
        }

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;

        case 'h':
            usage ();
            exit (SUCCESS);
            break;

        case 'v':
            version ();
            exit (SUCCESS);
            break;

        case 'x':
            *xml_filename = strdup (optarg);
            break;

        case 'w':
            *wigt = atof (optarg);
            break;
        case 'a':
            *awgt = atof (optarg);
            break;

        case 'p':
            *pswt_1 = atof (optarg);
            break;
        case 'q':
            *pswt_2 = atof (optarg);
            break;

        case 'n':
            *pswnt_1 = atoi (optarg);
            break;
        case 'o':
            *pswnt_2 = atoi (optarg);
            break;

        case 'r':
            *pswst_1 = atoi (optarg);
            break;
        case 's':
            *pswst_2 = atoi (optarg);
            break;

        case 't':
            *percent_slope = atof (optarg);
            break;
        case '?':
        default:
            snprintf (msg, sizeof (msg),
                      "Unknown option %s\n\n", argv[optind - 1]);
            ERROR_MESSAGE (msg, MODULE_NAME);
            usage ();
            return ERROR;
            break;
        }
    }

    /* Grab the boolean command line options */
    if (tmp_zeven_thorne_flag)
        *use_zeven_thorne_flag = true;
    else
        *use_zeven_thorne_flag = false;

    if (tmp_toa_flag)
        *use_toa_flag = true;
    else
        *use_toa_flag = false;

    if (tmp_include_tests_flag)
        *include_tests_flag = true;
    else
        *include_tests_flag = false;

    if (tmp_include_ps_flag)
        *include_ps_flag = true;
    else
        *include_ps_flag = false;

    if (tmp_verbose_flag)
        *verbose_flag = true;
    else
        *verbose_flag = false;

    /* Make sure the XML was specified */
    if (*xml_filename == NULL)
    {
        ERROR_MESSAGE ("XML input file is a required command line"
                       " argument\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Validate the input XML metadata file */
    if (validate_xml_file (*xml_filename) != SUCCESS)
    {
        /* Error messages already written */
        return ERROR;
    }

    /* Initialize the metadata structure */
    init_metadata_struct (xml_metadata);

    /* Parse the metadata file into our internal metadata structure; also
       allocates space as needed for various pointers in the global and band
       metadata */
    if (parse_metadata (*xml_filename, xml_metadata) != SUCCESS)
    {
        /* Error messages already written */
        return ERROR;
    }

    /* Assign the default values if not provided on the command line */
    if (strcmp(xml_metadata->global.satellite, "LANDSAT_8") == 0)
    {
        if (*wigt == NOT_SET)
            *wigt = wigt_l8_default;

        if (*awgt == NOT_SET)
            *awgt = awgt_l8_default;

        if (*pswt_1 == NOT_SET)
            *pswt_1 = pswt_1_l8_default;

        if (*pswt_2 == NOT_SET)
            *pswt_2 = pswt_2_l8_default;

        if (*pswnt_1 == NOT_SET)
            *pswnt_1 = pswnt_1_l8_default;

        if (*pswnt_2 == NOT_SET)
            *pswnt_2 = pswnt_2_l8_default;

        if (*pswst_1 == NOT_SET)
            *pswst_1 = pswst_1_l8_default;

        if (*pswst_2 == NOT_SET)
            *pswst_2 = pswst_2_l8_default;
    }
    else
    {
        if (*wigt == NOT_SET)
            *wigt = wigt_l47_default;

        if (*awgt == NOT_SET)
            *awgt = awgt_l47_default;

        if (*pswt_1 == NOT_SET)
            *pswt_1 = pswt_1_l47_default;

        if (*pswt_2 == NOT_SET)
            *pswt_2 = pswt_2_l47_default;

        if (*pswnt_1 == NOT_SET)
            *pswnt_1 = pswnt_1_l47_default;

        if (*pswnt_2 == NOT_SET)
            *pswnt_2 = pswnt_2_l47_default;

        if (*pswst_1 == NOT_SET)
            *pswst_1 = pswst_1_l47_default;

        if (*pswst_2 == NOT_SET)
            *pswst_2 = pswst_2_l47_default;
    }

    if (*percent_slope == NOT_SET)
        *percent_slope = percent_slope_default;


    /* ---------- Validate the parameters ---------- */
    if ((*wigt < 0.0) || (*wigt > 2.0))
    {
        ERROR_MESSAGE ("WIGT is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    if ((*awgt < -2.0) || (*awgt > 2.0))
    {
        ERROR_MESSAGE ("AWGT is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    if ((*pswt_1 < -2.0) || (*pswt_1 > 2.0))
    {
        ERROR_MESSAGE ("PSWT_1 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    if ((*pswt_2 < -2.0) || (*pswt_2 > 2.0))
    {
        ERROR_MESSAGE ("PSWT_2 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Only checking the low side here */
    if (*pswnt_1 < 0)
    {
        ERROR_MESSAGE ("PSWNT_1 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Only checking the low side here */
    if (*pswnt_2 < 0)
    {
        ERROR_MESSAGE ("PSWNT_2 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Only checking the low side here */
    if (*pswst_1 < 0)
    {
        ERROR_MESSAGE ("PSWST_1 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    /* Only checking the low side here */
    if (*pswst_2 < 0)
    {
        ERROR_MESSAGE ("PSWST_2 is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    if ((*percent_slope < 0.0) || (*percent_slope > 100.0))
    {
        ERROR_MESSAGE ("Percent Slope is out of range\n\n", MODULE_NAME);

        usage ();
        return ERROR;
    }

    return SUCCESS;
}
