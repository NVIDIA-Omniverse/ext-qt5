(: Name: copynamespace-7 :)
(: Written by: Carmelo Montanez :)
(: Description: Evaluates copy namespace declaration with value set to "no-preserve no-inherit" .:)
(: Use global variables and namespaces with prefixes.:)

declare copy-namespaces no-preserve,no-inherit;

declare variable $existingElement := <existingElement xmlns:existingNamespace="http://www.existingnamespace.com">{"Existing Content"}</existingElement>;
declare variable $new := <newElement xmlns = "http://www.mynamespace.com">{$existingElement}</newElement>;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in (in-scope-prefixes($new/existingElement))
order by exactly-one($var )ascending
return $var
