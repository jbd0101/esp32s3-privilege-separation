# ESP Privilege Separation with Enhanced Capabilities
## Authors : Jean-Christophe Bauduin, Lucas Roman
### Promoter : Pr. Ramin Sadre

This work was done as part of our master's thesis at the École Polytechnique de Louvain. 
The goal was to enhance the ESP privilege separation framework, by equipping it with more capabilities. We have implemented these new functionalities :
* Multitasking capabilities
  * Round Robin scheduling
  * Preemptive scheduling, with cooperative capabilities
  * Deep sleep mode
* Pipeline between the secure and non-secure world
* Secrets management with the nvs_flash library

## New Features Code
We have added most of our work as new components. The multitasking implementations and the secrets management are located in the [components/multitasking](components/multitasking) folder. The pipeline implementation is located in the [components/pipeline](components/pipeline) folder.

## Examples
For each new feature, we have created examples to demonstrate their use. To get started, please try out the [examples](examples). Each example has README with all setup instructions.

## Experiments
We have also conducted experiments to evaluate the performance of our new features. The results are available in the [experiment_results](experiment_results) folder. The jupyter notebook used to generate the graphs available in the same folder.