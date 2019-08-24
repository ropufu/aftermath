
# Make sure to either run ziggurat-numbers.R first, or load the corresponding environment.

#scale <- Rmpfr::mpfr("16777215", precBits = precision.bits); # Full 24 bits.
#scale <- Rmpfr::mpfr("2147483645", precBits = precision.bits); # Non-Mersenne.
#scale <- Rmpfr::mpfr("4294967295", precBits = precision.bits); # Full 32 bits.
#scale <- Rmpfr::mpfr("281474976710655", precBits = precision.bits); # Full 48 bits.
scale <- Rmpfr::mpfr("18446744073709551615", precBits = precision.bits); # Full 64 bits.

bits <- 80;
columns <- 8;
rows <- n.layers / columns;

dog <- function(x, t) {
  if (t == "u") type.name <- "uniform_type";
  if (t == "v") type.name <- "value_type";
  if (t == "e") type.name <- "expectation_type";
  
  suffix = "";
  if (t == "u") suffix = "ULL";
  if (t == "v") suffix = "L";
  if (t == "e") suffix = "L";
  
  if (t != "u") cat(type.name, "(", Rmpfr::formatDec(x, precBits = bits), suffix, ")", sep = "")
  else cat(type.name, "(", Rmpfr::formatMpfr(x, drop0trailing = TRUE), suffix, ")", sep = "");
}; # function(...)

tabulate <- function(x, t, name) {
  if (t == "u") type.name <- "uniform_type";
  if (t == "v") type.name <- "value_type";
  if (t == "e") type.name <- "expectation_type";
  
  tab.string <- strrep(" ", 4);
  cat(tab.string, tab.string, "static inline const ", type.name, " ", name, "[n_boxes] = {\n", sep = "");
  
  for (i in 1 : rows) {
    cat(tab.string, tab.string, tab.string, sep = "");
    for (j in 1 : columns) {
      k <- (i - 1) * columns + j;
      dog(x[k], t);
      if (k != n.layers) cat(", ");
    } # for (...)
    cat("\n");
  } # for (...)
  
  cat(tab.string, tab.string, "}; // ", name, "[...]\n", sep = "");
}; # function(...)

print.upscaled.low <- function() {
  temp <- ziggurat$low.probabilities;
  temp <- round(temp * scale);
  tabulate(temp, "u", "upscaled_low_probabilities");
}; # function(...)

print.upscaled.high <- function() {
  temp <- one - ziggurat$high.probabilities;
  temp <- round(temp * scale);
  tabulate(temp, "u", "upscaled_high_probabilities");
}; # function(...)

print.left.endpoints <- function() {
  temp <- ziggurat$left.endpoints;
  tabulate(temp, "v", "layer_left_endpoints");
}; # function(...)

print.bottom.endpoints <- function() {
  temp <- ziggurat$bottom.endpoints;
  tabulate(temp, "e", "layer_bottom_endpoints");
}; # function(...)

print.downscaled.widths <- function() {
  temp <- ziggurat$widths;
  temp <- temp / scale;
  tabulate(temp, "v", "downscaled_layer_widths");
}; # function(...)

print.downscaled.heights <- function() {
  temp <- ziggurat$heights;
  temp <- temp / scale;
  tabulate(temp, "e", "downscaled_layer_heights");
}; # function(...)

print.upscaled.low();
cat("\n");
print.upscaled.high();
cat("\n");
print.left.endpoints();
cat("\n");
print.downscaled.widths();
cat("\n");
print.bottom.endpoints();
cat("\n");
print.downscaled.heights();

cat # m=^.^=m~~
