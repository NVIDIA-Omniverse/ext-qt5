(:*******************************************************:)
(: Test: K2-SeqMINFunc-5                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:min() with an unsupported collation and one xs:anyURI value. :)
(:*******************************************************:)
min(xs:anyURI("str1"), "http://example.com/UNSUPPORTED_COLLATION")