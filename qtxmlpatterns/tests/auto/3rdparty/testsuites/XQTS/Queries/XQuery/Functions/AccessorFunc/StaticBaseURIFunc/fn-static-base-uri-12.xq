(: Name: fn-static-base-uri-12 :)
(: Description: Evaluation of fn:static-base-uri function using Used as argument to fn:string-length function. :)
(: Uses fn:string .:)

declare base-uri "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:string(fn:static-base-uri()))