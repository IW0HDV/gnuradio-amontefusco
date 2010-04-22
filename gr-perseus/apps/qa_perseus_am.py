#!/usr/bin/env python

#
# Demodulate an AM signal from the USRP or a recorded file.
# The file format must be 256 ksps, complex data.
# Excerpted from: http://www.ansr.org/kd7lmo/www.kd7lmo.net/ground_gnuradio_software.html

from gnuradio import gr, eng_notation
from gnuradio import audio
#from gnuradio import usrp
from gnuradio.eng_option import eng_option
from optparse import OptionParser

import sys
import math

import perseus

#
# return a gr.flow_graph
#
#def build_graph (file_name, freq_offset_khz):


class am_rx_block (gr.top_block):

    def __init__(self, file_name, sr, freq_offset_khz):
        gr.top_block.__init__(self)


        # Center IF frequency of down converter.
        #IF_freq = 10.7e6

        # Sampling rate of ADC on USRP
        adc_rate = 80e6

        # Decimation rate from USRP ADC to IF.
        perseus_decim = adc_rate / sr
        
        print "DECIM: %d" % perseus_decim

        # Calculate the sampling rate of the Perseus
        if_rate = adc_rate / perseus_decim

        print "IF sample rate:: %d" % if_rate

        # Decimate the IF sampling rate down by 125 to 62.5 kS/s
        if_decim = 2
        demod_rate = if_rate / if_decim

        print "Demod sample rate:: %d" % demod_rate
        
        audio_decimation = 2
        audio_rate = demod_rate / audio_decimation


        print "AUDIO RATE: %f\n" % audio_rate

        # Channelize the signal of interest.
        channel_coeffs = gr.firdes.low_pass (1.0,                # gain            
                                             if_rate,            # samplig freq    
                                             8000,               # cutoff_freq     
                                             1000,               # transition_width
                                             gr.firdes.WIN_HANN) # window_type     

        print "len(channel_coeffs) = ", len(channel_coeffs)

        # Tune to the desired frequency.
        # Construct a FIR filter with the given taps and a composite frequency translation 
        # that shifts center_freq down to zero Hz. 
        # The frequency translation logically comes before the filtering operation.
        self.ddc = gr.freq_xlating_fir_filter_ccf (if_decim,        # decimation,   
                                                   channel_coeffs,  # taps,         
                                                   freq_offset_khz, # center_freq,  
                                                   if_rate)         # sampling_freq 

        # Demodule with classic sqrt (I*I + Q*Q)
        self.magblock = gr.complex_to_mag()

        # Scale the audio
        self.volumecontrol = gr.multiply_const_ff(1.0)

        # Low pass filter the demodulator output

        # gain:              overall gain of filter (typically 1.0) 
        # sampling_freq:     sampling freq (Hz) 
        # cutoff_freq:       center of transition band (Hz) 
        # transition_width:  width of transition band (Hz). The normalized width of the transition 
        #                    band is what sets the number of taps required. 
        #                    Narrow --> more taps 
        # window_type:       What kind of window to use. Determines maximum 
        #                    attenuation and passband ripple. 
        # beta:              parameter for Kaiser window (6.6 default)

        audio_coeffs = gr.firdes.low_pass (1.0,                # gain
                                           demod_rate,         # samplig freq
                                           9e3,                # cutoff_freq
                                           4e3,                # transition_width
                                           gr.firdes.WIN_HANN) # window_type
        print "len(audio_coeffs) = ", len(audio_coeffs)

        # FIR filter with float input, float output and float taps. 
        # input: float; output: float
        self.audio_filter = gr.fir_filter_fff (audio_decimation, audio_coeffs)

        # sound card as final sink
        self.audio_sink = audio.sink(int(audio_rate),
                                     'plughw:0,0',
                                     False)  # ok_to_block


        # Signal source is assumed to be 256 kspb / complex data stream.
        if (file_name == "perseus"):
            try:
                # Select Perseus
                self.src = perseus.source_c (sr, "PERSEUS_0", True)
            except:
                raise Exception("PY: Perseus not found")

        else:
            self.src = gr.file_source (gr.sizeof_gr_complex, file_name)


        self.src.tune (7450000.0)

        # now wire it all together
        self.connect (self.src, self.ddc)
        self.connect (self.ddc, self.magblock)
        self.connect (self.magblock, self.volumecontrol)
        self.connect (self.volumecontrol, self.audio_filter)
        self.connect (self.audio_filter, (self.audio_sink, 0))
        
        #return fg

def main (args):
    nargs = len (args)
    if nargs == 2:
        filename = args[0]
        freq_offset_khz = float (args[1]) * 1e3
    else:
        sys.stderr.write ("usage: am_rcv_file {file_name,'usrp'} freq_offset_KHz\n")
        sys.exit (1)

    try:
        tb = am_rx_block(filename, 125000, freq_offset_khz)
        print "PY: >>>>>>\n   BEFORE RUNNING\n>>>>>>\n"

        tb.run()
    except KeyboardInterrupt:
        pass
    except:
        print "OUCH !"

if __name__ == '__main__':
    main (sys.argv[1:])

