(:*******************************************************:) 
(: Test: fn-except-node-args-009.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: processing-instruction node & node      :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external; 
(: insert-end :) 
 
$input-context/processing-instruction() except $input-context/bib/book[2]/title 
