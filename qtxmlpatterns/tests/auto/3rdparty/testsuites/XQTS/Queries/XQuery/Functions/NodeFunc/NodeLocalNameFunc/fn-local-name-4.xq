(: Name: fn-local-name-4:)
(: Description: Evaluation of the fn:local-name function argument set to empty sequence.:)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:count(fn:local-name(()))
