(:*******************************************************:)
(: Test: K-SeqExprCast-336                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a xs:dateTime with UTC timezone to xs:date. :)
(:*******************************************************:)

		   xs:date(xs:dateTime("2002-11-23T22:12:23.867-00:00")) eq xs:date("2002-11-23Z")
	