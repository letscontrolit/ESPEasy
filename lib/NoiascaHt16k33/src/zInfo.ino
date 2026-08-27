/*
  
    14 Segment
    
        A
    F K L M B
       G H
    E N O P C
        D
        
        
   7 Segment
      A 
    F   B
      G 
    E   C
      D   
      
      
      
print --> write(einzelnes Zeichen) --> writeLowLevel(wohin, bitmask)
      
      
open tasks   
  - a method to write a character to a specific position
  - replace writeLowLevel with a public version (including usage of _lastcharacter & displaybuffer)
  - type_t of displaybuffer and _lastcharacter one or two bytes as needed   
   
      
      
Noiasca_ht16k33                         base class - not useable
      Noiasca_ht16k33_hw_7              7 segments 8 digits
           Noiasca_ht16k33_hw_7_4_c     7 segments 4 digits, colon
      Noiasca_ht16k33_hw_14             14 segments 8 digits
           Noiasca_ht16k33_hw_14_4      14 segments 4 digits only
           Noiasca_ht16k33_hw_14_ext    14 segments 8 digits, extended support for scroll text
      
 */ 