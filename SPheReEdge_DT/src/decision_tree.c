#include <decision_tree.h>

void score(double * input, double * output) {
    double var0[2];
    if (input[3] <= 0.677356630563736) {
        if (input[4] <= 0.6612456142902374) {
            if (input[1] <= 0.22745202481746674) {
                if (input[0] <= 0.3157369792461395) {
                    if (input[5] <= 0.6352496147155762) {
                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                } else {
                    if (input[4] <= 0.48064446449279785) {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    }
                }
            } else {
                if (input[4] <= 0.4265211224555969) {
                    if (input[3] <= 0.6032598912715912) {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    } else {
                        if (input[5] <= 0.6378143727779388) {
                            if (input[2] <= 0.5078626275062561) {
                                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.3424830436706543) {
                        if (input[5] <= 0.637847363948822) {
                            if (input[3] <= 0.6747831404209137) {
                                if (input[5] <= 0.37534672021865845) {
                                    if (input[5] <= 0.3688902258872986) {
                                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[5] <= 0.32945650815963745) {
                            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[1] <= 0.4415092319250107) {
                                if (input[1] <= 0.43762075901031494) {
                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            }
        } else {
            if (input[5] <= 0.8588614463806152) {
                if (input[4] <= 0.6697684228420258) {
                    if (input[4] <= 0.6681309342384338) {
                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                } else {
                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            } else {
                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
            }
        }
    } else {
        if (input[2] <= 0.3235933184623718) {
            if (input[1] <= 0.3348183035850525) {
                if (input[5] <= 0.7479561567306519) {
                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                } else {
                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            } else {
                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
            }
        } else {
            if (input[5] <= 0.6761582493782043) {
                if (input[2] <= 0.3927912712097168) {
                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                } else {
                    if (input[4] <= 0.29605674743652344) {
                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    } else {
                        if (input[1] <= 0.4916292428970337) {
                            if (input[1] <= 0.27564653754234314) {
                                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 0.47562210261821747) {
                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[4] <= 0.5655306279659271) {
                    if (input[0] <= 0.2833077311515808) {
                        if (input[2] <= 0.48001880943775177) {
                            if (input[5] <= 0.7641226351261139) {
                                if (input[0] <= 0.044118279591202736) {
                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= 0.2502827048301697) {
                                        if (input[3] <= 0.8565589785575867) {
                                            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[5] <= 0.7278647124767303) {
                                                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[1] <= 0.2629448473453522) {
                                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                if (input[2] <= 0.4078456610441208) {
                                    if (input[5] <= 0.8152151703834534) {
                                        if (input[2] <= 0.37456221878528595) {
                                            if (input[1] <= 0.3227735012769699) {
                                                if (input[4] <= 0.44109418988227844) {
                                                    if (input[4] <= 0.4361084848642349) {
                                                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[4] <= 0.476348340511322) {
                                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[5] <= 0.7934464514255524) {
                                                if (input[2] <= 0.3916102647781372) {
                                                    if (input[1] <= 0.29136018455028534) {
                                                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[1] <= 0.25596781075000763) {
                                                        if (input[4] <= 0.4761538505554199) {
                                                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[3] <= 0.8541955649852753) {
                                                                if (input[5] <= 0.7883047759532928) {
                                                                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                                } else {
                                                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                }
                                                            } else {
                                                                memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    } else {
                                                        if (input[5] <= 0.7696683406829834) {
                                                            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            } else {
                                                if (input[3] <= 0.7921452224254608) {
                                                    if (input[5] <= 0.8009737432003021) {
                                                        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    } else {
                                        memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.3886323571205139) {
                            memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var0, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            }
        }
    }
    memcpy(output, var0, 2 * sizeof(double));
}
