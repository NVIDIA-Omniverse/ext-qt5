(:*******************************************************:)
(: Test: K-QuantExprWithout-98                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No 'at' declaration is allowed in 'some'-quantification. :)
(:*******************************************************:)
some $a at $p in (1, 2) satisfies $a