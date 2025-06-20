// srl x10 x22, x9  srl= R-type = upcode: 0110011, funct3: 101, funct7: 0000000
// = lw x9, 0(x10) lw = I-type = upcode: 0000011, funct3: 010 beq x9, x10, ELSE
// beq = SB-type = upcode: 1100011, funct3: 000 xor x9, x10, x22 xor = R-type =
// upcode: 0110011, funct3: 100, funct7: 0000000 sub x9, x10, x9 sub = R-type =
// upcode: 0110011, funct3: 000, funct7: 0100000 ELSE: addi x10, x10, 4 addi =
// I-type = upcode: 0010011, funct3: 000 = 000000000100(4 imm) 01010(x10)
// 000(funct3) 01010(x10) 0010011(opcode) sw x9, 0(x10) sw = S-type = upcode:
// 0100011, funct3: 010 = 0000000(0 immi) 01001(x9) 01010(x10) 010(funct3)
// 00000(0 immi) 0100011(opcode)

#include <ctype.h> // Para isspace
#include <stdio.h>
#include <stdlib.h> // Para atoi, strdup
#include <string.h>

typedef struct {
  const char *nome;
  const char *valor_binario;
} MapeamentoRegulador;

// Tabela com os mapeamentos de registradores RISC-V
MapeamentoRegulador tabela_registradores[] = {
    {"x0", "00000"},  {"zero", "00000"}, {"x1", "00001"},  {"ra", "00001"},
    {"x2", "00010"},  {"sp", "00010"},   {"x3", "00011"},  {"gp", "00011"},
    {"x4", "00100"},  {"tp", "00100"},   {"x5", "00101"},  {"t0", "00101"},
    {"x6", "00110"},  {"t1", "00110"},   {"x7", "00111"},  {"t2", "00111"},
    {"x8", "01000"},  {"s0", "01000"},   {"fp", "01000"},  {"x9", "01001"},
    {"s1", "01001"},  {"x10", "01010"},  {"a0", "01010"},  {"x11", "01011"},
    {"a1", "01011"},  {"x12", "01100"},  {"a2", "01100"},  {"x13", "01101"},
    {"a3", "01101"},  {"x14", "01110"},  {"a4", "01110"},  {"x15", "01111"},
    {"a5", "01111"},  {"x16", "10000"},  {"a6", "10000"},  {"x17", "10001"},
    {"a7", "10001"},  {"x18", "10010"},  {"s2", "10010"},  {"x19", "10011"},
    {"s3", "10011"},  {"x20", "10100"},  {"s4", "10100"},  {"x21", "10101"},
    {"s5", "10101"},  {"x22", "10110"},  {"s6", "10110"},  {"x23", "10111"},
    {"s7", "10111"},  {"x24", "11000"},  {"s8", "11000"},  {"x25", "11001"},
    {"s9", "11001"},  {"x26", "11010"},  {"s10", "11010"}, {"x27", "11011"},
    {"s11", "11011"}, {"x28", "11100"},  {"t3", "11100"},  {"x29", "11101"},
    {"t4", "11101"},  {"x30", "11110"},  {"t5", "11110"},  {"x31", "11111"},
    {"t6", "11111"}};

// struct com nome,tipo e formato de cada instrucao RISC-V
typedef struct {
  const char *nome;
  const char *opcode;
  const char *funct3;
  const char *funct7;
  const char *tipo_formato;
} Instrucao;

// Tabela com os dados das intrucoes RISC-V direcionadas a dupla
Instrucao tabela_instrucoes[] = {
    {"srl", "0110011", "101", "0000000", "R"},
    {"lw", "0000011", "010", NULL, "I"},
    {"beq", "1100011", "000", NULL, "SB"},
    {"xor", "0110011", "100", "0000000", "R"},
    {"sub", "0110011", "000", "0100000", "R"},
    {"addi", "0010011", "000", NULL, "I"},
    {"sw", "0100011", "010", NULL, "S"},
};

// Estrutura para armazenar saltos ou loops
typedef struct {
  char nome[50];
  int endereco; // endereço de memoria onde ocorre
} EntradaTabelaSimbolos;

// Tabela para armazenar todos os rotulos encontrados na primeira passagem
EntradaTabelaSimbolos tabela_simbolos[100];
int contador_simbolos = 0; // contagem de rotulos

// Remover espaços em branco (inicio/fim), quebras de linha e comentarios de uma
// string. Retorna um ponteiro para o inicio da string limpa.
char *limpar_linha(char *linha) {
  char *comentario = strchr(linha, '#');
  if (comentario) {
    *comentario = '\0'; // Remove tudo a partir do '#'
  }

  // Remove espaços/tabs/quebras de linha do final
  int i = strlen(linha) - 1;
  while (i >= 0 && isspace((unsigned char)linha[i])) {
    linha[i--] = '\0';
  }

  // Remove espaços/tabs do início
  char *inicio_limpo = linha;
  while (*inicio_limpo && isspace((unsigned char)*inicio_limpo)) {
    inicio_limpo++;
  }
  return inicio_limpo;
}

// Retorna o valor binário de 5 bits de um registrador (ex: "x10" -> "01010")
const char *obter_binario_registrador(const char *nome_reg) {
  for (int i = 0;
       i < sizeof(tabela_registradores) / sizeof(MapeamentoRegulador); i++) {
    if (strcmp(tabela_registradores[i].nome, nome_reg) == 0) {
      return tabela_registradores[i].valor_binario;
    }
  }
  fprintf(stderr, "Erro: Registrador desconhecido '%s'\n", nome_reg);
  return NULL;
}

// retorna formato da instreucao
const Instrucao *obter_detalhes_instrucao(const char *nome_instr) {
  for (int i = 0; i < sizeof(tabela_instrucoes) / sizeof(Instrucao); i++) {
    if (strcmp(tabela_instrucoes[i].nome, nome_instr) == 0) {
      return &tabela_instrucoes[i];
    }
  }
  fprintf(stderr, "Erro: Instrução '%s' não suportada.\n", nome_instr);
  return NULL;
}

// converte decimal - binario
void converter_dec_para_bin(int decimal, char *binario_str, int largura) {
  unsigned int valor_abs;
  if (decimal < 0) {
    valor_abs = (unsigned int)((1 << largura) + decimal); // Complemento de dois
  } else {
    valor_abs = (unsigned int)decimal;
  }

  for (int i = largura - 1; i >= 0; i--) {
    binario_str[largura - 1 - i] = ((valor_abs >> i) & 1) ? '1' : '0';
  }
  binario_str[largura] = '\0';
}
// coleta de saltos e loops
void primeira_leitura(FILE *arquivo_assembly, int *endereco_atual) {
  char linha[256];
  *endereco_atual = 0; // Começamos no endereço 0

  // Voltar para o início do arquivo, caso já tenha sido lido antes
  fseek(arquivo_assembly, 0, SEEK_SET);

  while (fgets(linha, sizeof(linha), arquivo_assembly)) {
    char *linha_processada = limpar_linha(linha);
    if (strlen(linha_processada) == 0)
      continue; // Pula linha vazia

    // Verifica se a linha começa com um rótulo (termina com ':')
    char *fim_rotulo = strchr(linha_processada, ':');
    if (fim_rotulo) {
      *fim_rotulo = '\0'; // "Corta" o rótulo da instrução

      // Adiciona o rótulo e seu endereço à tabela de símbolos
      strcpy(tabela_simbolos[contador_simbolos].nome, linha_processada);
      tabela_simbolos[contador_simbolos].endereco = *endereco_atual;
      contador_simbolos++;

      // Avança o ponteiro da linha para o que vem depois do salto ou loop
      linha_processada = fim_rotulo + 1;
      linha_processada =
          limpar_linha(linha_processada); // Limpa novamente caso haja espaços
      if (strlen(linha_processada) == 0)
        continue; // Era apenas um salto/loop na linha
    }

    // Se a linha não for apenas um rótulo(ELse, Loop, Exit), é uma instrução,
    // então incrementamos o endereço Cada instrução RISC-V tem 4 bytes
    *endereco_atual += 4;
  }
}

// Segunda leitura do arquivo, pra converter em binario

void segunda_leitura(FILE *arquivo_assembly) {
  char linha[256];
  int endereco_atual = 0; // Reinicia o contador de endereço

  // Voltar para o início do arquivo novamente para a segunda passagem
  fseek(arquivo_assembly, 0, SEEK_SET);

  while (fgets(linha, sizeof(linha), arquivo_assembly)) {
    char *linha_original_para_erro =
        strdup(linha); // Copia para mensagens de erro
    char *linha_processada = limpar_linha(linha);
    if (strlen(linha_processada) == 0) {
      free(linha_original_para_erro);
      continue;
    }

    // Verifica e pula rótulos (já tratados na primeira passagem, mas ainda na
    // linha)
    char *fim_rotulo = strchr(linha_processada, ':');
    if (fim_rotulo) {
      linha_processada = fim_rotulo + 1;
      linha_processada = limpar_linha(linha_processada);
      if (strlen(linha_processada) == 0) {
        free(linha_original_para_erro);
        continue;
      }
    }

    // Processa e controi a instrução binaria
    char *token =
        strtok(linha_processada, " \t,()"); // Separa o nome da instrução
    if (!token) {                           // Linha vazia ou mal formatada
      free(linha_original_para_erro);
      continue;
    }

    const Instrucao *info_instr = obter_detalhes_instrucao(token);
    if (!info_instr) { // Instrução não encontrada na tabela
      free(linha_original_para_erro);
      endereco_atual += 4;
      continue;
    }

    char instrucao_binaria[33]; // 32 bits + '\0'

    // definições de cada tipo de instrução

    if (strcmp(info_instr->tipo_formato, "R") == 0) {
      // Formato R-Type: funct7 | rs2 | rs1 | funct3 | rd | opcode
      char *rd_str = strtok(NULL, " \t,");
      char *rs1_str = strtok(NULL, " \t,");
      char *rs2_str = strtok(NULL, " \t,");

      const char *rd_bin = obter_binario_registrador(rd_str);
      const char *rs1_bin = obter_binario_registrador(rs1_str);
      const char *rs2_bin = obter_binario_registrador(rs2_str);

      if (!rd_bin || !rs1_bin || !rs2_bin) {
        fprintf(stderr, "Erro de registrador no R-Type da linha: %s",
                linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      // Concatena os pedaços para formar a instrução binária de 32 bits
      snprintf(instrucao_binaria, sizeof(instrucao_binaria), "%s%s%s%s%s%s",
               info_instr->funct7, rs2_bin, rs1_bin, info_instr->funct3, rd_bin,
               info_instr->opcode);

    } else if (strcmp(info_instr->tipo_formato, "I") == 0) {
      // Formato I-Type: immediate[11:0] | rs1 | funct3 | rd | opcode (Usado por
      // lw e addi)

      char *rd_str, *rs1_str, *imediato_str;
      int valor_imediato;

      if (strcmp(info_instr->nome, "lw") == 0) {
        rd_str = strtok(NULL, " \t,");       // Ex: "x9"
        imediato_str = strtok(NULL, " \t("); // Ex: "0" (offset)
        rs1_str = strtok(NULL, " \t)");      // Ex: "x10" (base register)
        valor_imediato = atoi(imediato_str);
      } else {                                // addi
        rd_str = strtok(NULL, " \t,");        // Ex: "x10"
        rs1_str = strtok(NULL, " \t,");       // Ex: "x10"
        imediato_str = strtok(NULL, " \t\n"); // Ex: "4" (immediate value)
        valor_imediato = atoi(imediato_str);
      }

      const char *rd_bin = obter_binario_registrador(rd_str);
      const char *rs1_bin = obter_binario_registrador(rs1_str);
      char bin_imediato[13]; // 12 bits + '\0'
      converter_dec_para_bin(valor_imediato, bin_imediato, 12);

      if (!rd_bin || !rs1_bin) {
        fprintf(stderr, "Erro de registrador no I-Type da linha: %s",
                linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      // Monta a instrução
      snprintf(instrucao_binaria, sizeof(instrucao_binaria), "%s%s%s%s%s",
               bin_imediato, rs1_bin, info_instr->funct3, rd_bin,
               info_instr->opcode);

    } else if (strcmp(info_instr->tipo_formato, "S") == 0) {
      // Formato S-Type: immediate[11:5] | rs2 | rs1 | funct3 | immediate[4:0] |
      // opcode  (Usado por sw)
      char *rs2_str = strtok(NULL, " \t,"); // Ex: "x9" (valor a ser armazenado)
      char *offset_str = strtok(NULL, " \t("); // Ex: "0" (offset)
      char *rs1_str = strtok(NULL, " \t)");    // Ex: "x10" (registrador base)

      int valor_offset = atoi(offset_str);
      const char *rs2_bin = obter_binario_registrador(rs2_str);
      const char *rs1_bin = obter_binario_registrador(rs1_str);

      char bin_offset[13]; // 12 bits para o offset
      converter_dec_para_bin(valor_offset, bin_offset, 12);

      // Fragmenta o offset nos pedaços esperados pelo formato S-Type
      char imm_11_5[8];
      strncpy(imm_11_5, bin_offset, 7);
      imm_11_5[7] = '\0'; // Bits 11 a 5
      char imm_4_0[6];
      strncpy(imm_4_0, bin_offset + 7, 5);
      imm_4_0[5] = '\0'; // Bits 4 a 0

      if (!rs1_bin || !rs2_bin) {
        fprintf(stderr, "Erro de registrador no S-Type da linha: %s",
                linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      // Monta a instrução
      snprintf(instrucao_binaria, sizeof(instrucao_binaria), "%s%s%s%s%s%s",
               imm_11_5, rs2_bin, rs1_bin, info_instr->funct3, imm_4_0,
               info_instr->opcode);

    } else if (strcmp(info_instr->tipo_formato, "SB") == 0) {
      // Formato SB-Type: imm[12|10:5] | rs2 | rs1 | funct3 | imm[4:1|11] |
      // opcode (Usado por beq)

      char *rs1_str = strtok(NULL, " \t,");             // Ex: "x9"
      char *rs2_str = strtok(NULL, " \t,");             // Ex: "x10"
      char *rotulo_destino_str = strtok(NULL, " \t\n"); // Ex: "ELSE"

      const char *rs1_bin = obter_binario_registrador(rs1_str);
      const char *rs2_bin = obter_binario_registrador(rs2_str);

      // Encontra o endereço do rótulo na tabela de símbolos
      int endereco_rotulo = -1;
      for (int i = 0; i < contador_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, rotulo_destino_str) == 0) {
          endereco_rotulo = tabela_simbolos[i].endereco;
          break;
        }
      }

      if (endereco_rotulo == -1) {
        fprintf(stderr,
                "Erro: Rótulo de destino '%s' não encontrado na linha: %s",
                rotulo_destino_str, linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      // Calcula o offset: (Endereço do Rótulo - Endereço da Instrução Atual)
      // O offset de branch é um valor assinado e sempre um múltiplo de 2.
      // O bit menos significativo (bit 0) é implícito e deve ser 0.
      int offset = (endereco_rotulo - endereco_atual);
      if (offset % 2 != 0) {
        fprintf(
            stderr,
            "Erro: Offset do branch '%d' não é um múltiplo de 2 na linha: %s",
            offset, linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      char bin_offset_composto[13]; // 12 bits efetivos + '\0' (representam
                                    // offset/2)
      converter_dec_para_bin(offset / 2, bin_offset_composto, 12);

      // Fragmenta o offset nos pedaços específicos do formato SB-Type
      char imm_12[2];
      imm_12[0] = bin_offset_composto[0];
      imm_12[1] = '\0'; // Bit 12
      char imm_10_5[7];
      strncpy(imm_10_5, bin_offset_composto + 2, 6);
      imm_10_5[6] = '\0'; // Bits 10 a 5
      char imm_4_1[5];
      strncpy(imm_4_1, bin_offset_composto + 8, 4);
      imm_4_1[4] = '\0'; // Bits 4 a 1
      char imm_11[2];
      imm_11[0] = bin_offset_composto[1];
      imm_11[1] = '\0'; // Bit 11

      if (!rs1_bin || !rs2_bin) {
        fprintf(stderr, "Erro de registrador no SB-Type da linha: %s",
                linha_original_para_erro);
        free(linha_original_para_erro);
        endereco_atual += 4;
        continue;
      }

      // Monta a instrução
      snprintf(instrucao_binaria, sizeof(instrucao_binaria), "%s%s%s%s%s%s%s%s",
               imm_12, imm_10_5, rs2_bin, rs1_bin, info_instr->funct3, imm_4_1,
               imm_11, info_instr->opcode);
    }

    // Imprime a instrução binária gerada
    printf("%s\n", instrucao_binaria);

    endereco_atual += 4; // Move para o endereço da próxima instrução
    free(linha_original_para_erro);
  }
}

int main() {
  FILE *arquivo_assembly;
  const char *nome_arquivo_entrada =
      "assembly.asm"; // Nome do seu arquivo Assembly

  // abre o arquivo Assembly
  arquivo_assembly = fopen(nome_arquivo_entrada, "r");
  if (arquivo_assembly == NULL) {
    perror("Erro ao abrir o arquivo Assembly. Certifique-se de que "
           "'assembly.asm' existe.");
    return 1;
  }

  int endereco_passagem1;
  primeira_leitura(arquivo_assembly, &endereco_passagem1);

  printf("Saltos ou Loops encontrados: \n\n");
  for (int i = 0; i < contador_simbolos; i++) {
    printf("  '%s' -> Endereço: 0x%04x\n", tabela_simbolos[i].nome,
           tabela_simbolos[i].endereco);
  }
  printf("\n");

  printf("Codigo Binario:\n\n");
  segunda_leitura(arquivo_assembly);

  // Fecha o arquivo
  fclose(arquivo_assembly);

  printf("\nMontagem concluida com sucesso!\n");
  return 0;
}