/*---------------------------------------------------------------
 * foreign data-wrapper for SheetReader
 *
 * Original author: Joel Ziegler <cody14@freenet.de>
 *---------------------------------------------------------------
 */


CREATE FUNCTION pg-sheet-fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
/*
CREATE FUNCTION pg-sheet-fdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
*/
CREATE FOREIGN DATA WRAPPER pg-sheet-fdw
  HANDLER pg-sheet-fdw_handler
/*  VALIDATOR pg-sheet-fdw_validator*/;
