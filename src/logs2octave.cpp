#include <stdio.h>
#include <limits.h>
#include "ECR.hpp"

void help_logs2octave() {
  printf("logs2octave -- Create Octave .m file to visualize logs.\n");
  printf(" -h : Help.\n");
  printf(" -l : Name of log file to process (required).\n");
  printf(" -r : Log file contains PHY receive metrics.\n");
  printf(" -t : Log file contains PHY transmit parameters.\n");
  printf(" -i : Log file contains interferer transmit parameters.\n");
  printf(" -c : Log file contains CRTS rx data.\n");
  printf(" -C : Log file contains CRTS tx data.\n");
  printf(" -N : Total number of repetitions for this scenario (default: 1)\n");
  printf(" -n : The repetition number for this scenario (required if -N is given)\n");
}

int main(int argc, char ** argv){

  char log_file[PATH_MAX]; 
  char output_file[PATH_MAX];
  char multi_file[PATH_MAX];

  enum log_t { PHY_RX_LOG = 0, PHY_TX_LOG, NET_RX_LOG, NET_TX_LOG, INT_TX_LOG };

  // default log type is PHY RX metrics
  log_t log_type = PHY_RX_LOG;

  strcpy(log_file, "logs/bin/");
  strcpy(output_file, "logs/octave/");
  strcpy(multi_file, "logs/octave/");

  unsigned int totalNumReps = 1;
  unsigned int repNumber = 1;

  // Option flags
  int l_opt = 0;
  int n_opt = 0;

  int d;
  while((d = getopt(argc, argv, "hl:rticCN:n:")) != EOF){  
    switch (d) {
    case 'h':
      help_logs2octave();
      return 0;
    case 'l': {

      // If log files use subdirectories, create them if they don't exist
      char * subdirptr = strrchr(optarg, '/');
      if (subdirptr) {
        char subdirs[1000];
        // Get the names of the subdirectories
        strncpy(subdirs, optarg, subdirptr - optarg);
        subdirs[subdirptr - optarg] = '\0';
        char mkdir_cmd[1000];
        strcpy(mkdir_cmd, "mkdir -p ./logs/octave/");
        strcat(mkdir_cmd, subdirs);
        // Create them
        system(mkdir_cmd);
      }

      strcat(log_file, optarg); 
      strcat(log_file, ".log");
      strcat(output_file, optarg);
      strcat(output_file, ".m");  
      strcat(multi_file, optarg);
      char *ptr_ = strrchr(multi_file, (int) '_');
      if (ptr_)
          multi_file[ ptr_ - multi_file] = '\0';
      strcat(multi_file, ".m");
      l_opt = 1;      
     break;
    }
    case 'r':
      break;
    case 't':
      log_type = PHY_TX_LOG;
      break;
    case 'i':
      log_type = INT_TX_LOG;
      break;
    case 'c':
      log_type = NET_RX_LOG;
      break;
    case 'C':
      log_type = NET_TX_LOG;
      break;
    case 'N': 
      totalNumReps = atoi(optarg);
      break;
    case 'n': 
      repNumber = atoi(optarg);             
      n_opt = 1;
      break;
    }
  }

  // Check that log file names were given
  if (!l_opt)
  {
    printf("Please give -l option.\n\n");
    help_logs2octave();
    return 1;
  }
  // Check that -n option is given as necessary
  if (totalNumReps>1 && !n_opt)
  {
    printf("-n option is required whenever -N option is given");
    help_logs2octave();
    return 1;
  }

  printf("Log file name: %s\n", log_file);
  printf("Output file name: %s\n", output_file);

  FILE * file_in = fopen(log_file, "rb");
  FILE * file_out;
  if (totalNumReps==1)
  {
    file_out = fopen(output_file, "w");
  }
  else
  {
    // rewrite the multi-file for the first repetition
	if (repNumber == 1)
	  file_out = fopen(multi_file, "w");
    else
	  file_out = fopen(multi_file, "a");
  }

  struct ExtensibleCognitiveRadio::metric_s metrics = {};
  struct ExtensibleCognitiveRadio::rx_parameter_s rx_params = {};
  struct ExtensibleCognitiveRadio::tx_parameter_s tx_params = {};
  int i = 1;

  // handle log type
  switch(log_type)
  {
    // handle PHY RX log
    case PHY_RX_LOG:
    {
      fprintf(file_out, "phy_rx_t = [];\n");
      // metrics
      fprintf(file_out, "phy_rx_Control_valid = [];\n");
      fprintf(file_out, "phy_rx_Payload_valid = [];\n");
      fprintf(file_out, "phy_rx_EVM = [];\n");
      fprintf(file_out, "phy_rx_RSSI = [];\n");
      fprintf(file_out, "phy_rx_CFO = [];\n");
      fprintf(file_out, "phy_rx_num_syms = [];\n");    
      fprintf(file_out, "phy_rx_mod_scheme = [];\n");
      fprintf(file_out, "phy_rx_BPS = [];\n");
      fprintf(file_out, "phy_rx_fec0 = [];\n");
      fprintf(file_out, "phy_rx_fec1 = [];\n");
      // parameters
      fprintf(file_out, "phy_rx_numSubcarriers = [];\n");
      fprintf(file_out, "phy_rx_cp_len = [];\n");
      fprintf(file_out, "phy_rx_taper_len = [];\n");
      fprintf(file_out, "phy_rx_gain_uhd = [];\n");
      fprintf(file_out, "phy_rx_freq = [];\n");
      fprintf(file_out, "phy_rx_rate = [];\n");    

      while(fread((char*)&metrics,
                  sizeof(struct ExtensibleCognitiveRadio::metric_s), 1,
                  file_in)) {
        fread((char*)&rx_params,
              sizeof(struct ExtensibleCognitiveRadio::rx_parameter_s), 1,
              file_in);
        fprintf(file_out, "phy_rx_t(%i) = %li + %f;\n", i, 
                metrics.time_spec.get_full_secs(), 
                metrics.time_spec.get_frac_secs());
        // metrics
        fprintf(file_out, "phy_rx_Control_valid(%i) = %i;\n", i,
                metrics.control_valid);
        fprintf(file_out, "phy_rx_Payload_valid(%i) = %i;\n", i,
                metrics.payload_valid);
        fprintf(file_out, "phy_rx_EVM(%i) = %f;\n", i,
                metrics.stats.evm);
        fprintf(file_out, "phy_rx_RSSI(%i) = %f;\n", i,
                metrics.stats.rssi);
        fprintf(file_out, "phy_rx_CFO(%i) = %f;\n", i,
                metrics.stats.cfo);
        fprintf(file_out, "phy_rx_num_syms(%i) = %i;\n", i,
                metrics.stats.num_framesyms);    
        fprintf(file_out, "phy_rx_mod_scheme(%i) = %i;\n", i,
                metrics.stats.mod_scheme);
        fprintf(file_out, "phy_rx_BPS(%i) = %i;\n", i,
                metrics.stats.mod_bps);
        fprintf(file_out, "phy_rx_fec0(%i) = %i;\n", i,
                metrics.stats.fec0);
        fprintf(file_out, "phy_rx_fec1(%i) = %i;\n\n", i,
                metrics.stats.fec1);
        // parameters
        fprintf(file_out, "phy_rx_numSubcarriers(%i) = %u;\n", i,
                rx_params.numSubcarriers);
        fprintf(file_out, "phy_rx_cp_len(%i) = %u;\n", i,
                rx_params.cp_len);
        fprintf(file_out, "phy_rx_taper_len(%i) = %u;\n", i,
                rx_params.taper_len);
        fprintf(file_out, "phy_rx_gain_uhd(%i) = %f;\n", i,
                rx_params.rx_gain_uhd);
        fprintf(file_out, "phy_rx_freq(%i) = %f;\n", i,
                rx_params.rx_freq - rx_params.rx_dsp_freq);
        fprintf(file_out, "phy_rx_rate(%i) = %f;\n", i,
                rx_params.rx_rate);    
        i++;
      }
      // If appending to the multiFile,
      // then put the data into the next elements of the cell arrays
      if (totalNumReps>1)
      {
        fprintf(file_out,
                "phy_rx_t_all_repetitions{%u} = phy_rx_t;\n",
                repNumber);
        // metrics
        fprintf(file_out,
                "phy_rx_Control_valid_all_repetitions{%u}     = phy_rx_Control_valid;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_Payload_valid_all_repetitions{%u}     = phy_rx_Payload_valid;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_EVM_all_repetitions{%u}   = phy_rx_EVM;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_RSSI_all_repetitions{%u}  = phy_rx_RSSI;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_CFO_all_repetitions{%u}   = phy_rx_CFO;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_num_syms_all_repetitions{%u}  = phy_rx_num_syms;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_mod_scheme_all_repetitions{%u}    = phy_rx_mod_scheme;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_BPS_all_repetitions{%u}   = phy_rx_BPS;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_fec0_all_repetitions{%u}  = phy_rx_fec0;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_fec1_all_repetitions{%u}  = phy_rx_fec1;\n",
                repNumber);
        // parameters
        fprintf(file_out,
                "phy_rx_numSubcarriers_all_repetitions{%u}    = phy_rx_numSubcarriers;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_cp_len_all_repetitions{%u}    = phy_rx_cp_len;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_taper_len_all_repetitions{%u} = phy_rx_taper_len;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_gain_uhd_all_repetitions{%u}  = phy_rx_gain_uhd;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_freq_all_repetitions{%u}  = phy_rx_freq;\n",
                repNumber);
        fprintf(file_out,
                "phy_rx_rate_all_repetitions{%u}  = phy_rx_rate;\n",
                repNumber);

        // Delete redundant data
        fprintf(file_out, "clear phy_rx_t;\n");
        // metrics
        fprintf(file_out, "clear phy_rx_Control_valid;\n");
        fprintf(file_out, "clear phy_rx_Payload_valid;\n");
        fprintf(file_out, "clear phy_rx_EVM;\n");
        fprintf(file_out, "clear phy_rx_RSSI;\n");
        fprintf(file_out, "clear phy_rx_CFO;\n");
        fprintf(file_out, "clear phy_rx_num_syms;\n");    
        fprintf(file_out, "clear phy_rx_mod_scheme;\n");
        fprintf(file_out, "clear phy_rx_BPS;\n");
        fprintf(file_out, "clear phy_rx_fec0;\n");
        fprintf(file_out, "clear phy_rx_fec1;\n\n");
        // parameters
        fprintf(file_out, "clear phy_rx_numSubcarriers;\n");
        fprintf(file_out, "clear phy_rx_cp_len;\n");
        fprintf(file_out, "clear phy_rx_taper_len;\n");
        fprintf(file_out, "clear phy_rx_gain_uhd;\n");
        fprintf(file_out, "clear phy_rx_freq;\n");
        fprintf(file_out, "clear phy_rx_rate;\n");    
      }
      break;
    }

    // handle PHY TX log
    case PHY_TX_LOG:
    {
      struct timeval log_time;;

      fprintf(file_out, "phy_tx_t = [];\n");
      fprintf(file_out, "phy_tx_numSubcarriers = [];\n");
      fprintf(file_out, "phy_tx_cp_len = [];\n");
      fprintf(file_out, "phy_tx_taper_len = [];\n");
      fprintf(file_out, "phy_tx_gain_uhd = [];\n");
      fprintf(file_out, "phy_tx_gain_soft = [];\n");
      fprintf(file_out, "phy_tx_freq = [];\n");
      fprintf(file_out, "phy_tx_lo_freq = [];\n");
      fprintf(file_out, "phy_tx_dsp_freq = [];\n");
      fprintf(file_out, "phy_tx_rate = [];\n");    
      fprintf(file_out, "phy_tx_mod_scheme = [];\n");
      fprintf(file_out, "phy_tx_fec0 = [];\n");
      fprintf(file_out, "phy_tx_fec1 = [];\n\n");

      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1, 
                  file_in)){
        fread((char*)&tx_params,
              sizeof(struct ExtensibleCognitiveRadio::tx_parameter_s), 1,
              file_in);
        fprintf(file_out, "phy_tx_t(%i) = %li + 1e-6*%li;\n", i,
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "phy_tx_numSubcarriers(%i) = %u;\n", i,
                tx_params.numSubcarriers);
        fprintf(file_out, "phy_tx_cp_len(%i) = %u;\n", i,
                tx_params.cp_len);
        fprintf(file_out, "phy_tx_taper_len(%i) = %u;\n", i,
                tx_params.taper_len);
        fprintf(file_out, "phy_tx_gain_uhd(%i) = %f;\n", i,
                tx_params.tx_gain_uhd);
        fprintf(file_out, "phy_tx_gain_soft(%i) = %f;\n", i,
                tx_params.tx_gain_soft);
        fprintf(file_out, "phy_tx_freq(%i) = %f;\n", i,
                tx_params.tx_freq + tx_params.tx_dsp_freq);
        fprintf(file_out, "phy_tx_lo_freq(%i) = %f;\n", i,
                tx_params.tx_freq);
        fprintf(file_out, "phy_tx_dsp_freq(%i) = %f;\n", i,
                tx_params.tx_dsp_freq);
        fprintf(file_out, "phy_tx_rate(%i) = %f;\n", i,
                tx_params.tx_rate);    
        fprintf(file_out, "phy_tx_mod_scheme(%i) = %i;\n", i,
                tx_params.fgprops.mod_scheme);
        fprintf(file_out, "phy_tx_fec0(%i) = %i;\n", i,
                tx_params.fgprops.fec0);
        fprintf(file_out, "phy_tx_fec1(%i) = %i;\n\n", i,
                tx_params.fgprops.fec1);
        i++;
      }

      // If appending to the multiFile, then put the data into the next elements of the cell arrays
      if (totalNumReps>1)
      {
        fprintf(file_out,
                "phy_tx_t_all_repetitions{%u} = phy_tx_t;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_numSubcarriers_all_repetitions{%u} = phy_tx_numSubcarriers;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_cp_len_all_repetitions{%u} = phy_tx_cp_len;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_taper_len_all_repetitions{%u} = phy_tx_taper_len;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_gain_uhd_all_repetitions{%u} = phy_tx_gain_uhd;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_gain_soft_all_repetitions{%u} = phy_tx_gain_soft;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_freq_all_repetitions{%u} = phy_tx_freq;\n",
                repNumber);
        fprintf(file_out, 
                "phy_tx_lo_freq_all_repetitions{%u} = phy_tx_lo_freq;\n",
                repNumber);
        fprintf(file_out, 
                "phy_tx_dsp_freq_all_repetitions{%u} = phy_tx_dsp_freq;\n",
                repNumber);
        fprintf(file_out,
                "phy_tx_rate_all_repetitions{%u} = phy_tx_rate;\n",
                repNumber);
        fprintf(file_out, 
                "phy_tx_mod_scheme_all_repetitions{%u} = phy_tx_mod_scheme;\n",
                repNumber);
        fprintf(file_out, 
                "phy_tx_fec0_all_repetitions{%u} = phy_tx_fec0;\n",
                repNumber);
        fprintf(file_out, 
                "phy_tx_fec1_all_repetitions{%u} = phy_tx_fec1;\n\n",
                repNumber);

        // Delete redundant data
        fprintf(file_out, "clear phy_tx_t;\n");
        fprintf(file_out, "clear phy_tx_numSubcarriers;\n");
        fprintf(file_out, "clear phy_tx_cp_len;\n");
        fprintf(file_out, "clear phy_tx_taper_len;\n");
        fprintf(file_out, "clear phy_tx_gain_uhd;\n");
        fprintf(file_out, "clear phy_tx_gain_soft;\n");
        fprintf(file_out, "clear phy_tx_freq;\n");
        fprintf(file_out, "clear phy_tx_lo_freq;\n");
        fprintf(file_out, "clear phy_tx_dsp_freq;\n");
        fprintf(file_out, "clear phy_tx_rate;\n");    
        fprintf(file_out, "clear phy_tx_mod_scheme;\n");    
        fprintf(file_out, "clear phy_tx_fec0;\n");    
        fprintf(file_out, "clear phy_tx_fec1;\n");    
      }
      break;
    }

    // handle interferer tx logs
    case INT_TX_LOG:
    {
      struct timeval log_time;

      fprintf(file_out, "Int_tx_t = [];\n");
      fprintf(file_out, "Int_tx_freq = [];\n");

      float tx_freq;
      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1,
                  file_in)){
        fread((float*)&tx_freq, sizeof(float), 1, file_in);
        fprintf(file_out, "Int_tx_t(%i) = %li + 1e-6*%li;\n",  i,
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "Int_tx_freq(%i) = %f;\n\n",  i, tx_freq);
        i++;
      }

      // If appending to the multiFile, 
      // then put the data into the next elements of the cell arrays
      if (totalNumReps>1)
      {
        fprintf(file_out,
                "Int_tx_t_all_repetitions{%u} = Int_tx_t;\n",
                repNumber);
        fprintf(file_out,
                "Int_tx_freq_all_repetitions{%u} = Int_tx_freq;\n\n",
                repNumber);

        // Delete redundant data
        fprintf(file_out, "clear Int_tx_t;\n");
        fprintf(file_out, "clear Int_tx_freq;\n\n");
      }
      break;
    }

    // handle NET RX data logs
    case NET_RX_LOG:
    {
      struct timeval log_time;
      int bytes;
      int packet_num;
      fprintf(file_out, "net_rx_t = [];\n");
      fprintf(file_out, "net_rx_bytes = [];\n");
      fprintf(file_out, "net_rx_packet_num = [];\n");
      while(fread((struct timeval*)&log_time, sizeof(struct timeval), 1,
                  file_in)){
        fread((int*)&bytes, sizeof(int), 1, file_in);
        fread((int*)&packet_num, sizeof(int), 1, file_in);
        fprintf(file_out, "net_rx_t(%i) = %li + 1e-6*%li;\n",  i,
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "net_rx_bytes(%i) = %i;\n",  i, bytes);
        fprintf(file_out, "net_rx_packet_num(%i) = %i;\n", i, packet_num);
        i++;
      }    

      // If appending to the multiFile,
      // then put the data into the next elements of the cell arrays
      if (totalNumReps>1)
      {
        fprintf(file_out,
                "net_rx_t_all_repetitions{%u} = net_rx_t;\n",
                repNumber);
        fprintf(file_out,
                "net_rx_bytes_all_repetitions{%u} = net_rx_bytes;\n\n",
                repNumber);
        fprintf(file_out,
                "net_rx_packet_num_all_repetitions{%u} = net_rx_packet_num;\n\n",
                repNumber);

        // Delete redundant data
        fprintf(file_out, "clear net_rx_t;\n");
        fprintf(file_out, "clear net_rx_bytes;\n");
        fprintf(file_out, "clear net_rx_packet_num;\n\n");
      }
      break;
	}
    case NET_TX_LOG:
    {
      struct timeval log_time;
	  int bytes;
	  int packet_num;
      fprintf(file_out, "net_tx_t = [];\n");
      fprintf(file_out, "net_tx_bytes = [];\n");
      fprintf(file_out, "net_tx_packet_num = [];\n");
      while (fread((struct timeval *)&log_time, sizeof(struct timeval), 1,
                   file_in)) {
        fread((int *)&bytes, sizeof(int), 1, file_in);
        fread((int *)&packet_num, sizeof(int), 1, file_in);
        fprintf(file_out, "net_tx_t(%i) = %li + 1e-6*%li;\n", i,
                log_time.tv_sec, log_time.tv_usec);
        fprintf(file_out, "net_tx_bytes(%i) = %i;\n\n", i, bytes);
        fprintf(file_out, "net_tx_packet_num(%i) = %i;\n\n", i, packet_num);
        i++;
      }
  
      // If appending to the multiFile,
      // then put the data into the next elements of the cell arrays
      if (totalNumReps>1)
      {
        fprintf(file_out,
                "net_tx_t_all_repetitions{%u} = net_tx_t;\n",
                repNumber);
        fprintf(file_out,
                "net_tx_bytes_all_repetitions{%u} = net_tx_bytes;\n\n",
                repNumber);
        fprintf(file_out,
                "net_tx_packet_num_all_repetitions{%u} = net_tx_packet_num;\n\n",
                repNumber);

        // Delete redundant data
        fprintf(file_out, "clear net_tx_t;\n");
        fprintf(file_out, "clear net_tx_bytes;\n");
        fprintf(file_out, "clear net_tx_packet_num;\n\n");
      }
	}
  }

  fclose(file_in);
  fclose(file_out);
}
