(:*******************************************************:)
(: Test: K-QuantExprWithout-99                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: No 'at' declaration is allowed in 'every'-quantification. :)
(:*******************************************************:)
every $a at $p in (1, 2) satisfies $a