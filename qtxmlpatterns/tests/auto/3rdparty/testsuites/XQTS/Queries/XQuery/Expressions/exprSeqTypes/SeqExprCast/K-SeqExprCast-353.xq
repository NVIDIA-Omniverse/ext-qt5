(:*******************************************************:)
(: Test: K-SeqExprCast-353                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: '25:00:00' is an invalid lexical representation for xs:time; hour part can never be larger than 24. :)
(:*******************************************************:)
xs:time("25:01:00")