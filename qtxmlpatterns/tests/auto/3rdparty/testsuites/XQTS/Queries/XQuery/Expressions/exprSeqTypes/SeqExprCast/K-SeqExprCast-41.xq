(:*******************************************************:)
(: Test: K-SeqExprCast-41                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `" white space left alone" cast as xs:string eq xs:string(" white space left alone")`. :)
(:*******************************************************:)
"  white space	left alone" cast as xs:string eq 
					xs:string("  white space	left alone")