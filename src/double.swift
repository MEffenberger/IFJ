let d = readDouble()
if let d  // d is not nil, reading was successful, d inside is not Double? but Double 
{
  let double : Double = d
  write("hexadecimalne: ", double)
}
else      // d is nil
{
  let dbl : Double = d ?? 0.0
  write("nula hexadecimalne: ", dbl)
}


