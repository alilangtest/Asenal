(print (defun our-member (obj lst)
  (if (null lst)
    nil
    (if (eql (car lst) obj)
        lst
        (our-member obj (cdr lst))))))

(print (our-member 'b '(a b c)))

