function L = ngaussLikelihood(noise, mu, varsigma, y)

% NGAUSSLIKELIHOOD Likelihood of data under noiseless Gaussian noise model.

% NOISE


L = gaussianLikelihood(noise, mu, varsigma, y);