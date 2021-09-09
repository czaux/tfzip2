//Include the c file itself because some structs and definitions are only located in it
//and re-writing them could be inconsistent
#include "tfbzlib.h"
#include "IOUtils.h"
#include "../bzip2-1.0.6//bzlib_private.h"

typedef
struct {
	FILE*     handle;
	Char      buf[BZ_MAX_UNUSED];
	Int32     bufN;
	Bool      writing;
	bz_stream strm;
	Int32     lastErr;
	Bool      initialisedOk;
}
bzFile;

#define BZ_SETERR(eee)                    \
{                                         \
   if (bzerror != NULL) *bzerror = eee;   \
   if (bzf != NULL) bzf->lastErr = eee;   \
}

int TFZ2_bzRead (int*    bzerror,
	BZFILE* b,
	void*   buf,
	int     len,
	unsigned long long &readlen)
{
	Int32   n, ret;
	bzFile* bzf = (bzFile*)b;
	readlen = 0;
	BZ_SETERR(BZ_OK);

	if (bzf == NULL || buf == NULL || len < 0)
	{
		BZ_SETERR(BZ_PARAM_ERROR); return 0;
	};

	if (bzf->writing)
	{
		BZ_SETERR(BZ_SEQUENCE_ERROR); return 0;
	};

	if (len == 0)
	{
		BZ_SETERR(BZ_OK); return 0;
	};

	bzf->strm.avail_out = len;
	bzf->strm.next_out = (char* )buf;

	while (True) {

		if (ferror(bzf->handle))
		{
			BZ_SETERR(BZ_IO_ERROR); return 0;
		};

		if (bzf->strm.avail_in == 0 && !myfeof(bzf->handle)) {
			n = fread(bzf->buf, sizeof(UChar),
				BZ_MAX_UNUSED, bzf->handle);
			readlen += n;
			if (ferror(bzf->handle))
			{
				BZ_SETERR(BZ_IO_ERROR); return 0;
			};
			bzf->bufN = n;
			bzf->strm.avail_in = bzf->bufN;
			bzf->strm.next_in = bzf->buf;
		}

		ret = BZ2_bzDecompress(&(bzf->strm));

		if (ret != BZ_OK && ret != BZ_STREAM_END)
		{
			BZ_SETERR(ret); return 0;
		};

		if (ret == BZ_OK && myfeof(bzf->handle) &&
			bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
		{
			BZ_SETERR(BZ_UNEXPECTED_EOF); return 0;
		};

		if (ret == BZ_STREAM_END)
		{
			BZ_SETERR(BZ_STREAM_END);
			return len - bzf->strm.avail_out;
		};
		if (bzf->strm.avail_out == 0)
		{
			BZ_SETERR(BZ_OK); return len;
		};

	}

	return 0; /*not reached*/
}