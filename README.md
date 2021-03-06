ngx_http_txtset module

First directive: txtset

Description:

"txtset" is similar to "set" found in the http rewrite module, except that the value to store in the variable isn't provided directly in the directive call, but rather in a text file passed to the directive.  If the text file cannot be read, or the value read from the text file contains invalid characters, a default value passed to the directive will be loaded as the value for the variable.  The directive reads a maximum of 20 characters from the file as the value to set.  The variable value is cached for 10 seconds by default to prevent excessive disk reads to the text file.  This time can be changed with an optional fourth parameter to the directive.

Valid locations for usage of directive:  NGX_HTTP_SRV_CONF|NGX_HTTP_SIF_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF

Usage:

txtset $vartoset /path/to/txt/file "DefaultValue" cache_time;

$vartoset: the variable to be added

/path/to/txt/file: path to a text file containing the desired value to load into $vartoset

"DefaultValue": the value that $vartoset will take when reading the value from /path/to/txt/file fails or contains invalid characters
	Valid characters: (A-Z) (a-z) (0-9) - _

cache_time: optional fourth parameter to set the cache time of $vartoset.  Default is 10 seconds if argument is not provided.

Example of use case: 
dynamic document root directory for switching between various software revisions



Second directive: txtlower

Description:

txtlower takes the value of a given variable and stores the lowercase version of the variable in a new variable.

Valid locations for usage of directive: NGX_HTTP_SRV_CONF|NGX_HTTP_SIF_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF

Usage:

txtlower $input_var $output_var;

$input_var: variable containing the value to be lowercase

$output_var: variable to store lowercase value from $input_var
