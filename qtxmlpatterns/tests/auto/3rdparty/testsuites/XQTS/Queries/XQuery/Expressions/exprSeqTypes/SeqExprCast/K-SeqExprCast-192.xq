(:*******************************************************:)
(: Test: K-SeqExprCast-192                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of casting a negative xs:yearMonthDuration to xs:dayTimeDuration. :)
(:*******************************************************:)

			xs:string(xs:dayTimeDuration(xs:yearMonthDuration("-P543Y456M")))
			eq "PT0S"