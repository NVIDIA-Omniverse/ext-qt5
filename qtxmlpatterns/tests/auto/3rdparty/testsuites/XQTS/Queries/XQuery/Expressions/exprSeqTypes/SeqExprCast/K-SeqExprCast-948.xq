(:*******************************************************:)
(: Test: K-SeqExprCast-948                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:date as sourceType and xs:NOTATION should fail due to it involving xs:NOTATION. :)
(:*******************************************************:)
not(xs:date("2004-10-13") castable as xs:NOTATION)