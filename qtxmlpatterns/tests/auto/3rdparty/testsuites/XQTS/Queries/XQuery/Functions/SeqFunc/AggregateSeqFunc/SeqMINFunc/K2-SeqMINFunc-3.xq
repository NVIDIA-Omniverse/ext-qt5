(:*******************************************************:)
(: Test: K2-SeqMINFunc-3                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:min() on two different xs:anyURI values(#2). :)
(:*******************************************************:)
min((xs:anyURI("http://example.com/8"), xs:anyURI("http://example.com/4")))
            eq xs:anyURI("http://example.com/4")