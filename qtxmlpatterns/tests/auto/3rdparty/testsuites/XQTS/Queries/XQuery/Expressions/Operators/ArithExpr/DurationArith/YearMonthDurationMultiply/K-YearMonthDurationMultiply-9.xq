(:*******************************************************:)
(: Test: K-YearMonthDurationMultiply-9                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The multiplication operator is not available between xs:integer and xs:duration. :)
(:*******************************************************:)
3 * xs:duration("P1Y3M")