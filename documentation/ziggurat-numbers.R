
if ("Rmpfr" %in% rownames(installed.packages()) == FALSE) install.packages("Rmpfr");
rm(list = ls()); # Clear environment.

n.layers <- 512;
precision.bits = 128;
dist.name <- "exp";

## Constants.
missing <- Rmpfr::mpfr(NA, precBits = precision.bits);
zero <- Rmpfr::mpfr(0, precBits = precision.bits);
one <- Rmpfr::mpfr(1, precBits = precision.bits);
two.pi <- 2 * Rmpfr::Const("pi", prec = precision.bits);
log.two.pi <- log(two.pi);

## Known distributions.

# Mode of the distribution.
mode.norm <- zero;
mode.exp <- zero;

# Survival function of the distribution.
tail.norm <- function(x) Rmpfr::pnorm(x, lower.tail = FALSE);
tail.exp <- function(x) { if (x < 0) return(one) else return(exp(zero - x)); }

# Density function of the distribution.
density.norm <- function(x) exp(-x * x / 2) / sqrt(two.pi);
density.exp <- function(x) { if (x < 0) return(zero) else return(exp(zero - x)); }

# Inverse density function of the distribution.
inverse.density.a.norm <- function(y) -sqrt(-2 * log(zero + y) - log.two.pi);
inverse.density.b.norm <- function(y) sqrt(-2 * log(zero + y) - log.two.pi);
inverse.density.a.exp <- function(y) zero;
inverse.density.b.exp <- function(y) (-log(y));

if (dist.name == "norm") {
  mode <- mode.norm;
  tail <- tail.norm;
  density <- density.norm;
  inverse.density.a <- inverse.density.a.norm;
  inverse.density.b <- inverse.density.b.norm;
} else if (dist.name == "exp") {
  mode <- mode.exp;
  tail <- tail.exp; # Survival function of the distribution.
  density <- density.exp;
  inverse.density.a <- inverse.density.a.exp;
  inverse.density.b <- inverse.density.b.exp;
} else {
  stop("Distribution not recognized.");
} # if (...)

## Sanity checks.
if (!is.finite(n.layers) || n.layers != round(n.layers)) stop("Number of layers should be an integer.");
if (n.layers < 2) stop("There should be at least two layers.");

## Initialization.
max.density <- density(mode); # Highest value the density can assume.

# Calculates the overshoot of the first box to land above the mode.
unwind <- function(f1, record.steps = FALSE) {
  result = NULL;
  
  result$volume <- missing;
  result$overshoot <- missing;
  result$simple.coverage <- missing;
  result$tail.probability <- missing;
  result$coverage <- missing;
  
  if (record.steps) {
    result$simple.coverage.probabilities <- rep(missing, times = n.layers);
    result$low.probabilities <- rep(missing, times = n.layers);
    result$high.probabilities <- rep(missing, times = n.layers);
    result$left.endpoints <- rep(missing, times = n.layers);
    result$right.endpoints <- rep(missing, times = n.layers);
    result$bottom.endpoints <- rep(missing, times = n.layers);
    result$top.endpoints <- rep(missing, times = n.layers);
    result$widths <- rep(missing, times = n.layers);
    result$heights <- rep(missing, times = n.layers);
  } # if (...)
  
  f.this <- zero;
  f.next <- zero + f1; # Make sure we work with high precision.
  if (f.next > max.density) { result$overshoot <- zero + max.density * n.layers; return(result); } # Make sure we start below the max density.
  if (f.next <= zero) { result$overshoot <- zero - max.density; return(result); } # Make sure we start below the max density.
  
  a.next <- inverse.density.a(f.next);
  b.next <- inverse.density.b(f.next);
  w.next <- b.next - a.next;
  v <- (one - tail(a.next)) + (b.next - a.next) * f.next + tail(b.next); # Volume of the bottom layer.
  
  a.this <- a.next - (one - tail(a.next)) / f.next;
  b.this <- b.next + (tail(b.next)) / f.next;
  w.this <- b.this - a.this;
  
  result$volume <- v;
  
  if (record.steps) {
    result$simple.coverage.probabilities[1] <- w.next / w.this;
    result$low.probabilities[1] <- (a.next - a.this) / w.this;
    result$high.probabilities[1] <- (b.this - b.next) / w.this;
    result$left.endpoints[1] <- a.this;
    result$right.endpoints[1] <- b.this;
    result$bottom.endpoints[1] <- f.this;
    result$top.endpoints[1] <- f.next;
    result$widths[1] <- w.this;
    result$heights[1] <- f.next - f.this;
  } # if (...)
  
  overshoot.penalty <- missing; # Penalty for early termination.
  for (i in 2 : n.layers) {
    a.this <- a.next;
    b.this <- b.next;
    w.this <- w.next;
    f.this <- f.next;
    
    f.next <- f.this + (v / w.this);
    
    # The case when x1 is too close to the mode has to be handled separately.
    if (f.next >= max.density) {
      a.next <- mode;
      b.next <- mode;
      overshoot.penalty <- zero + (n.layers - i) * (f.next - f.this);
    } # if (...)
    else {
      a.next <- inverse.density.a(f.next);
      b.next <- inverse.density.b(f.next);
    } # else (...)
    
    w.next <- b.next - a.next;
    
    if (record.steps) {
      result$simple.coverage.probabilities[i] <- w.next / w.this;
      result$low.probabilities[i] <- (a.next - a.this) / w.this;
      result$high.probabilities[i] <- (b.this - b.next) / w.this;
      result$left.endpoints[i] <- a.this;
      result$right.endpoints[i] <- b.this;
      result$bottom.endpoints[i] <- f.this;
      result$top.endpoints[i] <- f.next;
      result$widths[i] <- w.this;
      result$heights[i] <- f.next - f.this;
    } # if (...)
    
    if (!is.na(overshoot.penalty)) break;
  } # for (...)
  
  if (is.na(overshoot.penalty)) overshoot.penalty <- zero;
  
  result$overshoot <- (f.next - max.density) + overshoot.penalty;
  if (record.steps) {
    result$simple.coverage <- sum(result$simple.coverage.probabilities) / n.layers;
    result$tail.probability <- (one - result$simple.coverage.probabilities[1]) / n.layers;
    result$coverage <- 1 / (n.layers * v);
  } # if (...)
  return(result);
}; # unwind (...)

## Set up bounds for initialization.
upper.bound <- max.density / 2; # Initial guess for f1.
while (unwind(upper.bound)$overshoot > 0) upper.bound <- upper.bound / 2;
lower.bound <- upper.bound;
upper.bound <- 2 * upper.bound;

## Main loop.
count.iterations <- 0;
f1 <- max.density;
while (TRUE) {
  if (f1 == (lower.bound + upper.bound) / 2) break; # Precision limit reached.
  
  f1 <- (lower.bound + upper.bound) / 2;
  df <- unwind(f1)$overshoot;
  if (df < 0) lower.bound <- f1;
  if (df > 0) upper.bound <- f1;
  if (df == 0) break; # Almost surely impossible.
  
  count.iterations <- count.iterations + 1;
  if (count.iterations > 1000) stop("Too bad!");
} # while (...)

ziggurat <- unwind(upper.bound, record.steps = TRUE);
cat # m=^.^=m~~
