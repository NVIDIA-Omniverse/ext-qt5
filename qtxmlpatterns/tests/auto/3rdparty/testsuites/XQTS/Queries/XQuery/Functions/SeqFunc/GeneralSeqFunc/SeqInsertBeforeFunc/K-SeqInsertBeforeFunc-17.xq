(:*******************************************************:)
(: Test: K-SeqInsertBeforeFunc-17                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Apply a predicate to the result of fn:insert-before(). :)
(:*******************************************************:)
(insert-before((1, current-time(), 3), 1, (4, 5, 6))[last()] treat as xs:integer) eq 3