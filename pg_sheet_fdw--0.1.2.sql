/*---------------------------------------------------------------
 * foreign data-wrapper for SheetReader
 *
 * Original author: Joel Ziegler <cody14@freenet.de>
 *---------------------------------------------------------------
 */


CREATE FUNCTION pg_sheet_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
/*
CREATE FUNCTION pg-sheet-fdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;
*/
CREATE FOREIGN DATA WRAPPER pg_sheet_fdw
  HANDLER pg_sheet_fdw_handler
/*  VALIDATOR pg_sheet_fdw_validator*/;
