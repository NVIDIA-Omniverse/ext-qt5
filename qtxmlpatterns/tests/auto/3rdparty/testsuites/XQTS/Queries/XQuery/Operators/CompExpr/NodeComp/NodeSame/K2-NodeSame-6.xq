(:*******************************************************:)
(: Test: K2-NodeSame-6                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Compare two empty sequences, that are tricky to infer at compile time. :)
(:*******************************************************:)
empty(zero-or-one(<e/>/*) is zero-or-one(<e/>/*))